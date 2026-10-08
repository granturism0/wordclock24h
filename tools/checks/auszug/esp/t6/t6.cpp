/* Pruefstand F.5i (C23 / L207, Entscheidung vom 07.10.2026): /api/stm32_log liefert UTF-8 mit
 * JSON-Escapen statt ISO-8859-1.
 *
 * Uebersetzt den ECHTEN Code aus http.cpp (extract.py): die Modus-Konstanten,
 * utf8_sequence_len(), http_escape_text(), http_send_escaped(), http_api_stm32_log(). Der
 * Logring ist eine Attrappe mit den Zeilen unten; http_send() sammelt die Antwort, deren Rumpf
 * in die Datei aus argv[1] geht (run.sh prueft sie danach mit Python).
 *
 * Die vier Faelle aus dem Abnahmesatz C23, je in einer eigenen Zeile:
 *   ISO-8859-1-Umlaut (E4, FC) und Gradzeichen (B0)  => UTF-8 (C3 A4, C3 BC, C2 B0)
 *   Anfuehrungszeichen                                => \"
 *   Rueckstrich                                       => \\
 *   Steuerzeichen 0x01 und 0x1F                       => \u0001, \u001f
 * dazu: \t \r \n wie bisher, eine schon UTF-8-kodierte Zeile bleibt wortgleich (keine Doppel-
 * kodierung), DEL (7F) bleibt roh (in JSON erlaubt).
 *
 * Gegenprobe (DIR-014): Gegen ESP 3.2.25 schlagen Umlaut- und Steuerzeichenzeile an, und Python
 * weist die ganze Antwort ab.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#define FS(x) x
#define uint_fast8_t unsigned int                                       /* Zielbreite, L256 */
class String { std::string s; public: String (unsigned v) : s (std::to_string (v)) {} const char * c_str () const { return s.c_str (); } };
static std::string out;
static void http_send (const char * s) { out += s; }
static void http_send (String s) { out += s.c_str (); }
static void http_flush (void) {}
static const char * zeilen[] = {
    "Temperatur 21\xb0" "C gem\xe4ss F\xfchler",
    "Ticker \"Hallo\"",
    "Pfad C:\\wc",
    "Steuer\x01zeichen\x1f",
    "Tab\tCR\rLF\n",
    "schon UTF-8: \xc3\xa4\xc3\xb6\xc3\xbc",
    "~\x7f",
};
static uint16_t     stm32_log_get_count (void) { return sizeof zeilen / sizeof zeilen[0]; }
static const char * stm32_log_get_line (uint16_t i) { return zeilen[i]; }
static void         http_send_escaped (const char * s, uint_fast8_t mode);
#include "c23_code.inc"

int main (int argc, char ** argv)
{
    static const char * soll[] = {
        "\"Temperatur 21\xc2\xb0" "C gem\xc3\xa4ss F\xc3\xbchler\"",
        "\"Ticker \\\"Hallo\\\"\"",
        "\"Pfad C:\\\\wc\"",
        "\"Steuer\\u0001zeichen\\u001f\"",
        "\"Tab\\tCR\\rLF\\n\"",
        "\"schon UTF-8: \xc3\xa4\xc3\xb6\xc3\xbc\"",
        "\"~\x7f\"",
    };
    static const char * name[] = { "ISO-8859-1-Umlaut und Gradzeichen => UTF-8", "Anfuehrungszeichen => \\\"", "Rueckstrich => \\\\",
                                   "Steuerzeichen 01, 1F => \\u00XX", "\\t \\r \\n wie bisher", "UTF-8 bleibt wortgleich (keine Doppelkodierung)", "DEL bleibt roh" };
    int fails = 0, n = sizeof soll / sizeof soll[0];
    printf ("Pruefstand F.5i / C23 (%s)\n", BEZEICHNUNG);
    http_api_stm32_log ();
    size_t body = out.find ("\r\n\r\n");
    std::string b = body == std::string::npos ? out : out.substr (body + 4);
    for (int i = 0; i < n; i++)
    {
        bool ok = b.find (soll[i]) != std::string::npos;
        printf ("  [%s] %s\n", ok ? " ok " : "FEHL", name[i]);
        fails += ! ok;
    }
    if (argc > 1) { FILE * f = fopen (argv[1], "wb"); fwrite (b.data (), 1, b.size (), f); fclose (f); }
    printf ("Ergebnis: %d von %d Faellen bestanden\n", n - fails, n);
    return fails ? 1 : 0;
}
