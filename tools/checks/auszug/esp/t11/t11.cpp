/* Pruefstand Review F.6 A2: http_remote_stm32_filename_matches () -- Flashzweig, Legacy-Liste,
 * API-Liste und /api/remote_stm32_flash benutzen sie alle (F.1, F.3).
 *
 * Echter Code aus http.cpp (extract.py): http_build_stm32_default_filename () und die
 * Pruefung. ZIELTYPEN (L256): uint_fast8_t/uint_fast16_t wie am ESP 32 Bit.
 *
 * Namensraum: jeder Name, den der Builder ueber ALLE 4096 Hardwarekennungen (Bits 0..11) bilden
 * kann, dazu jeder dieser Namen mit "uc-", "wc24h-" und "wc12h-" vertauscht, dazu Unsinn. Jeder
 * Name wird gegen jede Kennung und gegen 65535 geprueft:
 *
 *   (1) BESTAND: Was die bisherige Regel durchliess (Laenge >= 6, ab Zeichen 6 steht der Filter),
 *       passt weiter -- insbesondere der Wechsel wc12h <-> wc24h. Nicht verschaerft.
 *   (2) UC: Der eigene Standardname passt bei jeder Kennung mit Filter und bekanntem Layout
 *       (wc24h, wc12h, uc), auch bei HW_UCLOCK
 *       (bis Review F.6 passte "uc-..." nie). Gegenprobe gegen ESP 3.2.25 und gegen F.1-F.5j.
 *   (3) NICHT GELOCKERT: Darueber hinaus passt nur, was mit dem Praefix des Geraets beginnt und
 *       direkt dahinter den Filter traegt -- kein uc-Name auf einer Wordclock, kein fremder Chip.
 *   (4) 65535 und Kennungen ohne Filter: nichts passt.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <set>
#define uint_fast8_t  unsigned int
#define uint_fast16_t unsigned int
#include VARSHDR
#define MAX_UPDATE_FILENAME_LEN 64
static uint_fast16_t hardware_configuration = 0xFFFF;
#include "a2_code.inc"

static const char * geraetepraefix (unsigned hw)
{
    switch (hw & HW_WC_MASK) { case HW_WC_24H: return "wc24h-"; case HW_WC_12H: return "wc12h-"; case HW_UCLOCK: return "uc-"; }
    return "";
}
static bool beginnt (const std::string & s, const std::string & p) { return s.compare (0, p.size (), p) == 0; }

int main (void)
{
    std::set<std::string> namen;
    long faelle = 0, f1 = 0, f2 = 0, f3 = 0, f4 = 0, uc_eigen = 0;

    for (unsigned hw = 0; hw < 4096; hw++)
    {
        char n[MAX_UPDATE_FILENAME_LEN]; const char * f;
        hardware_configuration = hw; http_build_stm32_default_filename (n, sizeof n, &f);
        if (f && n[0]) namen.insert (n);
    }
    std::set<std::string> alle = namen;
    for (auto & n : namen)
    {
        size_t s = n.find ("stm32");
        if (s == std::string::npos) continue;
        for (const char * p : { "uc-", "wc24h-", "wc12h-", "", "xx-", "wc24h_" }) alle.insert (std::string (p) + n.substr (s));
    }
    for (const char * j : { "", "a", "uc-", "wc24h-", "uc-stm32", "wc24h-stm32f4", "uc-stm32f411-", "stm32f411-sk6812-rgbw.hex" }) alle.insert (j);
    printf ("Pruefstand A2 / Praefix (%s): %zu Namen x 4097 Kennungen\n", BEZEICHNUNG, alle.size ());

    for (unsigned hw = 0; hw <= 4096; hw++)
    {
        unsigned h = hw == 4096 ? 0xFFFF : hw;
        char def[MAX_UPDATE_FILENAME_LEN]; const char * f;
        hardware_configuration = h; http_build_stm32_default_filename (def, sizeof def, &f);
        std::string filt = f ? f : "", pre = h == 0xFFFF ? "" : geraetepraefix (h);

        for (auto & a : alle)
        {
            hardware_configuration = h;
            bool ist  = http_remote_stm32_filename_matches (a.c_str ()) != 0;
            bool alt  = f && a.size () >= 6 && beginnt (a.substr (6), filt);
            bool eig  = f && ! pre.empty () && beginnt (a, pre) && a.size () >= pre.size () && beginnt (a.substr (pre.size ()), filt);
            faelle++;
            if (! f && ist)                       { if (f4++ < 3) printf ("  FEHL (4) hw=%04x '%s' passt ohne Filter\n", h, a.c_str ()); }
            else if (alt && ! ist)                { if (f1++ < 3) printf ("  FEHL (1) hw=%04x '%s' passte bisher, jetzt nicht\n", h, a.c_str ()); }
            else if (ist && ! alt && ! eig)       { if (f3++ < 3) printf ("  FEHL (3) hw=%04x '%s' zusaetzlich durchgelassen\n", h, a.c_str ()); }
            else if (eig && ! ist)                { if (f3++ < 3) printf ("  FEHL (3) hw=%04x '%s' eigenes Praefix + Filter, abgewiesen\n", h, a.c_str ()); }
        }
        if (f && ! pre.empty ())                                           /* unbekannte Layoutkennung: kein Praefix, (3) */
        {
            hardware_configuration = h;
            bool ok = http_remote_stm32_filename_matches (def) != 0;
            faelle++;
            if ((h & HW_WC_MASK) == HW_UCLOCK) uc_eigen++;
            if (! ok && f2++ < 3) printf ("  FEHL (2) hw=%04x eigener Standardname '%s' abgewiesen\n", h, def);
        }
    }
    printf ("  [%s] (1) Bestand: alles, was bisher passte, passt weiter (%ld Verstoesse)\n", f1 ? "FEHL" : " ok ", f1);
    printf ("  [%s] (2) eigener Standardname passt, darunter %ld uc-Kennungen (%ld Verstoesse)\n", f2 ? "FEHL" : " ok ", uc_eigen, f2);
    printf ("  [%s] (3) nichts darueber hinaus gelockert (%ld Verstoesse)\n", f3 ? "FEHL" : " ok ", f3);
    printf ("  [%s] (4) ohne Filter (65535) passt nichts (%ld Verstoesse)\n", f4 ? "FEHL" : " ok ", f4);
    printf ("Ergebnis: %ld Einzelpruefungen, %ld Verstoesse\n", faelle, f1 + f2 + f3 + f4);
    return (f1 + f2 + f3 + f4) ? 1 : 0;
}
