/* Attrappe fuer AKO.1: hasOverrun() wie im Core 3.1.2 -- liefert das Merkmal und LOESCHT es (uart.cpp:920). */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#define uint_fast8_t unsigned int
/* Flash-Ablage (Review E.6): PSTR/snprintf_P legen auf dem ESP nur das Format in den Flash; der Text ist byte-gleich. */
#define PSTR(s)     (s)
#define snprintf_P  snprintf
static uint32_t fake_ms = 1000;
static uint32_t millis (void) { return fake_ms; }
#define ESP_HEAP_LOG_INTERVAL 60000UL
static int has_calls = 0;
struct SerialStub {
    bool overrun = false; std::string out;
    bool hasOverrun (void) { has_calls++; bool r = overrun; overrun = false; return r; }
    void println (const char * s) { out += s; out += "\r\n"; }
    void flush (void) {}
} Serial;
struct EspStub { uint32_t getFreeHeap (void) { return 4294967295u; } uint32_t getMaxFreeBlockSize (void) { return 4294967295u; } } ESP;
static std::vector<std::string> logring;
static void stm32_log_append (const char * l) { logring.push_back (l); }
static uint32_t heap_low_line_take (void) { return 4294967295u; }   /* E3 I-1: kein Messpunkt seit der vorigen Zeile, low= = max= */
/* R1.1 I-5 (Paket 2026-10-15): die Heap-Zeile endet auf tcp=/nrt=; hier leer bzw. 0 - Gegenstand ist ovr= */
static size_t tcp_now_tcp_field (char * b, size_t n) { return n ? (size_t) snprintf (b, n, "tcp=0/0/0/0/0/0") : 0; }
static uint16_t http_no_request_timeouts_get (void) { return 0; }
#define HEAP_ZUSATZ " tcp=0/0/0/0/0/0 nrt=0"
/* C80 (Paket 2026-10-17-c80, Release I): loop () traegt die Messpunkte des Instruments (heapkarte.h). Hier ohne
 * Wirkung, wie mit ESP_HEAP_MAP 0 - Gegenstand dieses Pruefstands ist ovr=. */
#define HEAPKARTE_ANFANG()          do { } while (0)
#define HEAPKARTE_VOR()             do { } while (0)
#define HEAPKARTE_NACH(t)           do { } while (0)
#define HEAPKARTE_AUSGABE()         do { } while (0)
#include "ovr_var.inc"
#include "ovr_heap.inc"
static void poll (void)
{
#include "ovr_poll.inc"
}
static int fails = 0, faelle = 0;
static void fehl (const char * f, const std::string & w) { fails++; printf ("      FEHLER [%s]: %s\n", f, w.c_str ()); }
static std::string heapline (void) { fake_ms += 60000; Serial.out.clear (); size_t n = logring.size (); esp_heap_log (); return logring.size () > n ? logring.back () : ""; }
static void erwarte (const char * f, unsigned long ovr)
{
    faelle++;
    std::string s = "- heap free=4294967295 max=4294967295 ovr=" + std::to_string (ovr) + " low=4294967295";   /* E3: low= hinten */
    std::string l = heapline ();
    if (l != s && l != s + HEAP_ZUSATZ) fehl (f, "API-Ring: erwartet \"" + s + "\", bekam \"" + l + "\"");   /* R1.1: mit oder ohne Zusatz */
    if (Serial.out != s + "\r\n" && Serial.out != s + HEAP_ZUSATZ + "\r\n") fehl (f, "Mitschnitt: erwartet \"" + s + "\"");
}
int main (void)
{
    printf ("ovr (%s)\n", BEZEICHNUNG);
    poll (); erwarte ("kein Ueberlauf", 0);
    Serial.overrun = true; poll (); erwarte ("ein Ueberlauf", 1);
    poll (); poll (); erwarte ("Merkmal geloescht, kein Doppelzaehlen", 1);
    for (int k = 0; k < 5; k++) { Serial.overrun = true; poll (); }
    erwarte ("fuenf weitere", 6);
    int hc = has_calls; poll (); faelle++;
    if (has_calls != hc + 1) fehl ("je loop() eine Abfrage", "hasOverrun() nicht genau einmal je Durchlauf");
    /* Saettigung: Zaehler kurz vor dem Ende, die Abfrage muss trotzdem laufen (sonst bliebe das Merkmal stehen) */
#ifndef OVR_FEHLT
    esp_uart_overruns = 0xFFFFFFFEu;
#endif
    Serial.overrun = true; poll (); erwarte ("bis 4294967295", 4294967295ul);
    Serial.overrun = true; hc = has_calls; poll (); erwarte ("gesaettigt, kein Umlauf auf 0", 4294967295ul);
    faelle++;
    if (has_calls != hc + 1 || Serial.overrun) fehl ("gesaettigt", "Merkmal nicht mehr abgefragt/geloescht");
    /* Laengste Form passt ungekuerzt (67 Zeichen seit E3) -- geprueft im Fall darueber ueber den exakten Vergleich */
    printf ("  Faelle: %d\n", faelle);
    printf ("%s  ovr %s: %d Fehler\n", fails ? "FEHL" : "OK  ", BEZEICHNUNG, fails);
    return fails ? 1 : 0;
}
