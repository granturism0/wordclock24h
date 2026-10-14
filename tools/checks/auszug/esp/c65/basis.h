/* Gemeinsame Attrappen fuer die Pruefstaende c65a, c65b, c65m, c65n (Paket 2026-10-13, E3, design.md 18.7).
 * Nur so viel, wie die ausgeschnittenen Stuecke aus http.cpp, httpclient.cpp und ESP-uclock.ino brauchen.
 * millis () liefert unsigned long wie im Auszug; die Zeitwerte der Faelle bleiben unter 2^32 - ein Umlauf ist
 * hier nicht Gegenstand (kein AKR verlangt ihn), deshalb ist die Breite nicht angeglichen (vgl. L256).
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <algorithm>

class __FlashStringHelper;
#define F(s)        ((const __FlashStringHelper *) (s))
#define FS(s)       (s)
#define PSTR(s)     (s)
#define snprintf_P  snprintf
#define strncpy_P   strncpy
typedef const char * PGM_P;

struct String
{
    std::string s;
    String () {}
    String (const char * p) : s (p ? p : "") {}
    String (const __FlashStringHelper * p) : s ((const char *) p) {}
    String (const std::string & x) : s (x) {}
    explicit String (unsigned long v) : s (std::to_string (v)) {}
    explicit String (long v) : s (std::to_string (v)) {}
    explicit String (unsigned int v) : s (std::to_string (v)) {}
    explicit String (int v) : s (std::to_string (v)) {}
    bool        startsWith (const char * p) const { return s.compare (0, strlen (p), p) == 0; }
    String      substring (unsigned a) const { return a <= s.size () ? String (s.substr (a)) : String (); }
    String      substring (unsigned a, unsigned e) const { return a <= s.size () && e >= a ? String (s.substr (a, e - a)) : String (); }
    const char *c_str () const { return s.c_str (); }
    unsigned    length () const { return (unsigned) s.size (); }
    bool        operator== (const char * p) const { return s == p; }
    bool        operator!= (const char * p) const { return s != p; }
    int         indexOf (const char * p, unsigned from = 0) const { size_t r = s.find (p, from); return r == std::string::npos ? -1 : (int) r; }
    int         indexOf (const String & p, unsigned from = 0) const { return indexOf (p.c_str (), from); }
    String &    operator+= (const char * p) { s += p; return *this; }
    String &    operator+= (char c) { s += c; return *this; }
};
static inline String operator+ (const String & a, const char * b) { return String (a.s + b); }
static inline String operator+ (const String & a, const String & b) { return String (a.s + b.s); }

/* Zeit */
static unsigned long fake_ms = 0;
static unsigned long millis (void) { return fake_ms; }
static void delay (unsigned long ms) { fake_ms += ms; }
static void sdk_abgabe (void);                      /* Abgabepunkt: hier stellt der Treiber zu (je Pruefstand) */
static void yield (void) { sdk_abgabe (); }

/* Serial und Ring */
struct SerialAttrappe
{
    std::vector<std::string> lines; std::string cur;
    void print (const char * p) { cur += p; }
    void print (const __FlashStringHelper * p) { cur += (const char *) p; }
    void print (const String & p) { cur += p.s; }
    void print (char c) { cur += c; }
    void print (unsigned long v) { cur += std::to_string (v); }
    void print (int v) { cur += std::to_string (v); }
    void print (unsigned int v) { cur += std::to_string (v); }
    void println (const char * p) { cur += p; lines.push_back (cur); cur.clear (); }
    void println (const __FlashStringHelper * p) { println ((const char *) p); }
    void println (const String & p) { println (p.c_str ()); }
    void println () { lines.push_back (cur); cur.clear (); }
    void write (const uint8_t * p, size_t n) { cur.append ((const char *) p, n); }
    void flush () {}
} Serial;
static std::vector<std::string> ring;
void stm32_log_append (const char * l) { ring.push_back (l); }

/* ESP: Wertfolgen fuer getMaxFreeBlockSize () und getFreeHeap () */
struct EspAttrappe
{
    std::deque<uint32_t> blk_seq, heap_seq; uint32_t blk = 5000, heap = 6000; int blk_calls = 0;
    uint32_t getMaxFreeBlockSize () { blk_calls++; if (! blk_seq.empty ()) { blk = blk_seq.front (); blk_seq.pop_front (); } return blk; }
    uint32_t getFreeHeap () { if (! heap_seq.empty ()) { heap = heap_seq.front (); heap_seq.pop_front (); } return heap; }
    uint32_t getFlashChipRealSize () { return 4194304; }
} ESP;

/* Pruefprotokoll */
static int fails = 0, faelle = 0;
static void fall (const char * name, bool ok, const std::string & warum = "")
{
    faelle++;
    if (! ok) { fails++; printf ("      FEHLER [%s]: %s\n", name, warum.c_str ()); }
}
static int ende (const char * stand, const char * bez)
{
    printf ("  Faelle: %d\n", faelle);
    printf ("%s  %s %s: %d Fehler\n", fails ? "FEHL" : "OK  ", stand, bez, fails);
    return fails ? 1 : 0;
}
