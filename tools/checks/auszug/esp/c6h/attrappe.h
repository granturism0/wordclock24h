/* Attrappen fuer die Auszugs-Pruefstaende c6h, c46, c19 (Paket 2026-10-13, E1): Arduino-String,
 * Serial, F()/FS(), http_send, LittleFS, Bruecke. Nur so viel, wie die ausgeschnittenen Stuecke brauchen.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include VARSHDR

class __FlashStringHelper;
#define F(s)    ((const __FlashStringHelper *) (s))
#define FS(s)   String (F(s)).c_str ()

struct String
{
    std::string s;
    String () {}
    String (const char * p) : s (p ? p : "") {}
    String (const __FlashStringHelper * p) : s ((const char *) p) {}
    String (const std::string & x) : s (x) {}
    explicit String (unsigned int v) : s (std::to_string (v)) {}
    explicit String (int v) : s (std::to_string (v)) {}
    bool        startsWith (const char * p) const { return s.compare (0, strlen (p), p) == 0; }
    String      substring (unsigned a) const { return a <= s.size () ? String (s.substr (a)) : String (); }
    String      substring (unsigned a, unsigned e) const { return a <= s.size () ? String (s.substr (a, e - a)) : String (); }
    void        trim () { size_t a = s.find_first_not_of (" \t\r\n"); size_t e = s.find_last_not_of (" \t\r\n");
                          s = a == std::string::npos ? "" : s.substr (a, e - a + 1); }
    const char *c_str () const { return s.c_str (); }
    unsigned    length () const { return (unsigned) s.size (); }
    bool        operator== (const char * p) const { return s == p; }
    bool        operator!= (const char * p) const { return s != p; }
    int         indexOf (const char * p, unsigned from = 0) const { size_t r = s.find (p, from); return r == std::string::npos ? -1 : (int) r; }
    int         indexOf (const String & p, unsigned from = 0) const { return indexOf (p.c_str (), from); }
};

static std::string serial_out;
struct SerialAttrappe
{
    void print (const char * p) { serial_out += p; }
    void print (const __FlashStringHelper * p) { serial_out += (const char *) p; }
    void print (char c) { serial_out += c; }
    void print (unsigned long v) { serial_out += std::to_string (v); }
    void print (int v) { serial_out += std::to_string (v); }
    void print (unsigned int v) { serial_out += std::to_string (v); }
    template <class T> void println (T v) { print (v); serial_out += "\r\n"; }
    void println () { serial_out += "\r\n"; }
    void flush () {}
} Serial;

static std::string http_out;
void http_send (const char * s) { http_out += s; }
void http_send (String s) { http_out += s.s; }
void http_flush (void) {}
#define http_send_FS(x)     http_send(FS(x))

static void delay (unsigned long) {}

/* Anfrage-Parameter */
static std::map<std::string, std::string> params;
static char * http_get_param (const char * n)
{
    static char b[8][1100]; static int i; char * p = b[i++ % 8];
    snprintf (p, sizeof b[0], "%s", params.count (n) ? params[n].c_str () : ""); return p;
}

/* Wirkungen */
static int n_stm32_reset, n_rpc_reset_eeprom, n_format, n_remove;
static std::vector<std::pair<int, std::string>> strvar_sets;
static std::string strvar_vals[MAX_STR_VARIABLES];
static void stm32_reset (void) { n_stm32_reset++; }
unsigned int rpc (RPC_VARIABLE v) { if (v == RESET_EEPROM_RPC_VAR) n_rpc_reset_eeprom++; return 1; }
unsigned int set_numvar (NUM_VARIABLE, unsigned int) { return 1; }
unsigned int set_tm_var (TM_VARIABLE, TM *) { return 1; }
unsigned int set_strvar (STR_VARIABLE v, const char * x) { strvar_sets.push_back ({ (int) v, x }); strvar_vals[v] = x; return 1; }
STR_VAR * get_strvar (STR_VARIABLE v)
{
    static char b[MAX_STR_VARIABLES][128]; static STR_VAR s[MAX_STR_VARIABLES];
    snprintf (b[v], sizeof b[v], "%s", strvar_vals[v].c_str ()); s[v].str = b[v]; s[v].maxlen = 127; return &s[v];
}
int dayofweek (int, int, int) { return 0; }
static void update_progress_state (const char *, const char *) {}
static void update_progress_complete (const char *) {}

struct LittleFSAttrappe
{
    bool begin () { return true; }
    void end () {}
    bool format () { n_format++; return true; }
    bool remove (const char *) { n_remove++; return true; }
    bool exists (const char *) { return true; }
} LittleFS;

/* Kopfzeilen der Anfrage, Zeile fuer Zeile wie read_line () sie liefert */
static std::deque<std::string> kopfzeilen;
static bool read_line (String & line)
{
    if (kopfzeilen.empty ()) return false;
    line = String (kopfzeilen.front ()); kopfzeilen.pop_front (); return true;
}
struct ClientAttrappe { int available () { return 0; } int peek () { return -1; } int read () { return -1; } } http_client;
static void http_capture_request_user_agent (const String &) {}
