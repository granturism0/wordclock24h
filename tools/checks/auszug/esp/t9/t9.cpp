/* Pruefstand F.5d (C9k / L172): httpclient_read_header () mit Segment-Attrappe.
 * ZIELTYPEN (L256): millis () ist uint32_t wie am ESP; delay () rueckt die Uhr vor.
 * 7 Faelle: ein Segment, zwei Segmente (40 ms), Gegenstelle schliesst vor Segment 2 (Nachlauf),
 * abgerissener Kopf (=> -1, nicht 200), 404, dazu (A3) ein endloser Kopf flutend und tropfend:
 * -1 und hoechstens zwei Zeitgrenzen. Gegenprobe ESP 3.2.25: 5 Fehler; Stand F.5d ohne A3: 2 Fehler.
 */
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <vector>
static uint32_t now;                                       /* 32 Bit wie am ESP */
static uint32_t millis () { return now; }
static void delay (int ms) { now += ms; }
static int mystrnicmp (const char * a, const char * b, int n) { return strncasecmp (a, b, n); }
struct Seg { uint32_t at; std::string s; };
/* ENDLOS (A3): Nach den Segmenten liefert die Attrappe auf Wunsch unbegrenzt Kopfzeilen nach,
 * je ein Zeichen alle 'gap' ms -- gap 1 ist ein Server, der flutet, gap 1000 einer, der tropft.
 * NOTBREMSE: Ab 60 s Pruefstandzeit verstummt sie und schliesst; so endet auch die alte Fassung
 * (die keine Gesamtgrenze kennt) und schlaegt an, statt den Pruefstand anzuhalten. */
struct Fake {
  std::vector<Seg> segs; size_t si = 0, pos = 0; uint32_t close_at = 0xFFFFFFFF;
  uint32_t gap = 0, next = 0; size_t epos = 0; const char * endlos = "X-Endlos: 1\r\n";
  bool bremse () const { return gap && now >= 60000; }
  int available () { if (si < segs.size ()) return now >= segs[si].at ? 1 : 0; return gap && ! bremse () && now >= next ? 1 : 0; }
  int read () {
    if (! available ()) return -1;
    if (si < segs.size ()) { int c = (unsigned char) segs[si].s[pos++]; if (pos >= segs[si].s.size ()) { si++; pos = 0; next = now + gap; } return c; }
    int c = endlos[epos++]; if (! endlos[epos]) epos = 0; next = now + gap; return c; }
  int connected () { return now < close_at && ! bremse (); }
} client;
#include "c9k_code.inc"
static int fails, n;
static void fall (std::vector<Seg> segs, int soll_err, int soll_len, uint32_t close_at = 0xFFFFFFFF, uint32_t gap = 0)
{ client = Fake (); client.segs = segs; client.close_at = close_at; client.gap = gap; now = 0; httpclient_grace_left = HTTPCLIENT_PEER_GONE_GRACE; n++;
  int len = -7; int err = httpclient_read_header (&len);
  if (err != soll_err || (soll_err == 200 && len != soll_len)) { fails++; printf ("FEHL fall %d: err=%d len=%d (soll %d/%d) t=%u\n", n, err, len, soll_err, soll_len, (unsigned) now); }
  else if (gap && now > 2 * HTTPCLIENT_READ_TIMEOUT) { fails++; printf ("FEHL fall %d: endloser Kopf hielt %u ms an (Grenze %u)\n", n, (unsigned) now, 2u * HTTPCLIENT_READ_TIMEOUT); } }
int main ()
{
  std::string k1 = "HTTP/1.1 200 OK\r\nServer: x\r\n", k2 = "Content-Length: 42\r\n\r\n";
  fall ({{0, k1 + k2}}, 200, 42);                         /* ein Segment */
  fall ({{0, k1}, {40, k2}}, 200, 42);                    /* zwei Segmente, 40 ms Abstand (L172) */
  fall ({{0, k1}, {40, k2}}, 200, 42, 20);                /* Gegenstelle schliesst vor dem 2. Segment, Nachlauf reicht */
  fall ({{0, k1}}, -1, 0);                                /* Kopf reisst ab: Zeitgrenze, kein 200 */
  fall ({{0, "HTTP/1.1 404 Not Found\r\n\r\n"}}, 404, 0);
  fall ({{0, k1}}, -1, 0, 0xFFFFFFFF, 1);                 /* A3: endloser Kopf, flutend (1 Zeichen je ms) */
  fall ({{0, k1}}, -1, 0, 0xFFFFFFFF, 1000);              /* A3: endloser Kopf, tropfend (1 Zeichen je s) */
  printf ("%s (%s): %d Faelle, %d Fehler\n", fails ? "FEHLGESCHLAGEN" : "OK", BEZEICHNUNG, n, fails); return fails ? 1 : 0;
}
