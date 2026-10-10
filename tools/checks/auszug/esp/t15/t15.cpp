/* Pruefstand C53/L356 (Kopie esp-c54): Warten auf die erste Anfragezeile in http.cpp.
 *
 * Attrappe mit Core-3.1.2-Semantik, Zeit in Mikrosekunden:
 *  - ZUSTELLUNG nur an Abgabepunkten: delay () und das optimistic_yield (100) IN available ()
 *  - available (): erhebt getSize () ZUERST, gibt DANACH ab, wenn 0 (WiFiClient.cpp:246-257). Ein Segment,
 *    das in dieser Abgabe zugestellt wird, steckt im Puffer, der Rueckgabewert ist trotzdem 0
 *  - connected (): 0, sobald das FIN zugestellt ist (CLOSE_WAIT => state () CLOSED, WiFiClient.cpp:327-333,
 *    ClientContext.h:363-371) oder die Verbindung per RST weg ist (_error: _pcb = nullptr,
 *    ClientContext.h:619-628); der Puffer bleibt in beiden Faellen (ClientContext.h:594-615)
 *  - FIN nur nach allen Daten, in Reihenfolge (lwIP tcp_in.c: FIN nur bei seqno == rcv_nxt)
 *  - Der Code zwischen dem Ende von delay (1) und dem Erheben in available () kostet KOSTEN_US
 *
 * Gerätefall C53: Ein Client schickt die Anfrage und schliesst halb (nc -N, shutdown (SHUT_WR)); Daten und
 * FIN kommen zusammen. Faellt die Zustellung in das optimistic_yield von available (), sah der alte Stand
 * available () == 0 und connected () == 0, verwarf die Anfrage und zaehlte einen Abbruch.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <string>

static uint64_t now_us;
static const uint64_t KOSTEN_US = 20;                     /* Schleifenkopf, millis (), Vergleich */
static unsigned long millis () { return (unsigned long) (now_us / 1000); }
static void sdk ();
static void delay (unsigned long ms) { now_us += ms * 1000; sdk (); }
struct FakeSerial { void print (const char *) {} void println (const char * = "") {} void flush () {} } Serial;

struct Szenario {
  long daten_us = -1;                                     /* Zustellbar ab; -1 = keine Daten */
  long fin_us = -1;                                       /* -1 = kein FIN */
  long rst_us = -1;                                       /* -1 = kein RST */
};
static Szenario sz;

struct WiFiClient {
  std::string buf; bool fin = false, rst = false, gestoppt = false; int abgaben_in_available = 0;
  void pull () {
    if (sz.daten_us >= 0 && (uint64_t) sz.daten_us <= now_us && buf.empty () && ! fin) buf = "GET /api/device_ready HTTP/1.1\r\n";
    if (sz.fin_us >= 0 && (uint64_t) sz.fin_us <= now_us && (sz.daten_us < 0 || (uint64_t) sz.daten_us <= now_us)) fin = true;   /* in Reihenfolge */
    if (sz.rst_us >= 0 && (uint64_t) sz.rst_us <= now_us) rst = true; }
  int available () { now_us += KOSTEN_US; int r = (int) buf.size (); if (! r) { abgaben_in_available++; now_us += 100; pull (); } return r; }
  uint8_t connected () { if (rst || fin) return 0; return 1; }
  void stop () { gestoppt = true; }
};
static WiFiClient http_client;
static void sdk () { http_client.pull (); }

#include "defs.inc"

static int angekommen;
static void warte_auf_anfrage ()
{
  angekommen = 0;
#include "block.inc"
  angekommen = 1;
}

static int fails, n, checks;
#define PRUEF(cond, ...) do { checks++; if (! (cond)) { fails++; printf ("FEHL fall %d (%s): ", n, name); printf (__VA_ARGS__); printf ("\n"); } } while (0)

/* Ergebnis eines Laufs: angekommen, Zuwachs der beiden Zaehler, stop () gerufen */
struct Erg { int an; int to; int ab; bool stop; };
static Erg lauf (Szenario s, uint64_t start_us)
{
  sz = s; http_client = WiFiClient (); now_us = start_us;
  uint16_t t0 = http_no_request_timeouts, a0 = http_no_request_aborts;
  warte_auf_anfrage ();
  return { angekommen, http_no_request_timeouts - t0, http_no_request_aborts - a0, http_client.gestoppt };
}

static void fall (const char * name, Szenario s, int soll_an, int soll_to, int soll_ab)
{
  n++;
  Erg e = lauf (s, 1000000);
  PRUEF (e.an == soll_an, "angekommen=%d, erwartet %d", e.an, soll_an);
  PRUEF (e.to == soll_to && e.ab == soll_ab, "Zaehler Zeitueberschreitung +%d / Abbruch +%d, erwartet +%d / +%d", e.to, e.ab, soll_to, soll_ab);
  PRUEF (e.stop == ! soll_an, "stop () %s", e.stop ? "gerufen" : "nicht gerufen");
}

/* Ueber den Ankunftszeitpunkt fegen: 0 bis 3000 us nach dem Start in 10-us-Schritten */
static void feger (const char * name, bool mit_fin, int soll_an, int soll_ab)
{
  n++; int an = 0, ab = 0, gesamt = 0, stale = 0;
  for (long off = 0; off <= 3000; off += 10) {
    Szenario s; s.daten_us = 1000000 + off; if (mit_fin) s.fin_us = s.daten_us;
    Erg e = lauf (s, 1000000); gesamt++; an += e.an; ab += e.ab;
  }
  printf ("  (%s: %d Zeitpunkte, angekommen %d, als Abbruch gezaehlt %d)\n", name, gesamt, an, ab);
  PRUEF (an == (soll_an ? gesamt : 0), "%d von %d Anfragen angekommen", an, gesamt);
  PRUEF (ab == soll_ab, "%d als Abbruch gezaehlt, erwartet %d", ab, soll_ab);
}

int main ()
{
  { Szenario s; s.daten_us = 1000000 + 5000; fall ("Anfrage nach 5 ms, Verbindung bleibt offen", s, 1, 0, 0); }
  { Szenario s; s.daten_us = 1000000; fall ("Anfrage sofort da", s, 1, 0, 0); }
  { Szenario s; fall ("keine Anfrage, Verbindung bleibt offen (Chrome auf Vorrat)", s, 0, 1, 0); }
  { Szenario s; s.fin_us = 1000000 + 3000; fall ("FIN ohne Anfrage", s, 0, 0, 1); }
  { Szenario s; s.rst_us = 1000000 + 3000; fall ("RST ohne Anfrage (AbortController)", s, 0, 0, 1); }
  { Szenario s; s.daten_us = 1000000 + 3000; s.fin_us = 1000000 + 9000; fall ("Anfrage, FIN spaeter", s, 1, 0, 0); }
  { Szenario s; s.daten_us = 1000000 + 260000; fall ("Anfrage nach 260 ms", s, 0, 1, 0); }
  /* C53: Daten und FIN in derselben Zustellung, Zeitpunkt in das optimistic_yield von available () gelegt:
   * erster Durchlauf: Erheben bei +20 us, Abgabe bis +120 us, delay (1) endet bei 1001120 us; im zweiten
   * Durchlauf Erheben bei 1001140 us, Abgabe bis 1001240 us */
  { Szenario s; s.daten_us = 1001120 + 50; s.fin_us = s.daten_us; fall ("C53 Anfrage samt FIN in der Abgabe von available ()", s, 1, 0, 0); }
  { Szenario s; s.daten_us = 1000000 + 600; s.fin_us = s.daten_us; fall ("Anfrage samt FIN waehrend delay (1)", s, 1, 0, 0); }
  feger ("C53 Feger Anfrage samt FIN", true, 1, 0);
  feger ("Feger Anfrage ohne FIN", false, 1, 0);

  printf ("%s (%s): %d Faelle, %d Pruefungen, %d Fehler\n", fails ? "FEHLGESCHLAGEN" : "OK", BEZEICHNUNG, n, checks, fails);
  return fails ? 1 : 0;
}
