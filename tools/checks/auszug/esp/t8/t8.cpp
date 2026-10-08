/* Pruefstand F.5c (C25 / L270): http_flush () mit Attrappe fuer http_client (write/connected).
 * 4 Faelle: Browser weg, Verbindung steht ohne Fortschritt, Teilschreiben dann Abbau, alles
 * hinaus (keine Zeile). Geprueft wird die Verlustzeile im Logring: "gone=" und "fail=".
 * Gegenprobe ESP 3.2.25: 3 Fehler (Zeile ohne Zaehler).
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <string>
#define TCP_MSS 536
static int conn = 1; static size_t wmax = 0;
struct Client { size_t write (const uint8_t *, size_t n) { return n < wmax ? n : wmax; } int connected () { return conn; } } http_client;
struct SerialStub { void println (const char *) {} void flush () {} } Serial;
static std::string lastlog; static int nlog;
static void stm32_log_append (const char * l) { lastlog = l; nlog++; }
static void yield () {}
#include "c25_code.inc"
static int fails, n;
static void fall (int c, size_t w, const char * soll)
{ http_write_broken = 0; conn = c; wmax = w; lastlog = ""; n++;
  strcpy (http_response, "0123456789"); http_response_len = 10; http_flush ();
  if (soll ? lastlog.find (soll) == std::string::npos : lastlog != "") { fails++; printf ("FEHL fall %d: log='%s' soll enthaelt '%s'\n", n, lastlog.c_str (), soll ? soll : "(nichts)"); } }
int main ()
{
  fall (0, 0, "lost 10 gone=1 fail=0");     /* Browser weg */
  fall (1, 0, "lost 10 gone=1 fail=1");     /* Verbindung steht, nichts geht hinaus */
  fall (0, 4, "lost 6 gone=2 fail=1");      /* Teil geschrieben, dann Abbau */
  fall (1, 100, 0);                          /* alles geht hinaus: keine Zeile */
  printf ("%s (%s): %d Faelle, %d Fehler\n", fails ? "FEHLGESCHLAGEN" : "OK", BEZEICHNUNG, n, fails); return fails ? 1 : 0;
}
