/* Attrappe fuer L408: ein loop()-Abschnitt samt Heap-Zeile, hasOverrun() wie im Core 3.1.2 (liefert und loescht). */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#define uint_fast8_t unsigned int
#define PSTR(s)     (s)
#define snprintf_P  snprintf
static uint32_t fake_ms = 0;                     /* 32 Bit wie auf dem ESP */
static uint32_t millis (void) { return fake_ms; }
#define ESP_HEAP_LOG            1
#define ESP_HEAP_LOG_INTERVAL   60000UL
static int order_bad = 0;
struct SerialStub {
    bool overrun = false; bool polled = false; std::vector<std::string> out;
    bool hasOverrun (void) { polled = true; bool r = overrun; overrun = false; return r; }
    void println (const char * s) { if (! strncmp (s, "- heap ", 7) && ! polled) order_bad++; out.push_back (s); }
    void flush (void) {}
} Serial;
struct EspStub { uint32_t getFreeHeap (void) { return 1000; } uint32_t getMaxFreeBlockSize (void) { return 900; } } ESP;
static std::vector<std::string> logring;
static void stm32_log_append (const char * l) { logring.push_back (l); }
static void var_sync_check (void) {}
#include "z_var.inc"
#include "z_heap.inc"
static int fails = 0, faelle = 0;
static void fehl (const char * f, const std::string & w) { fails++; printf ("      FEHLER [%s]: %s\n", f, w.c_str ()); }
/* ein loop()-Durchlauf zur Zeit ms, mit oder ohne Ueberlauf seit der letzten Abfrage */
static void durchlauf (unsigned long ms, bool ovr)
{
    fake_ms = ms; Serial.overrun = ovr; Serial.polled = false; Serial.out.clear (); logring.clear ();
#include "z_loop.inc"
}
static std::string ovrzeile (void)
{
    std::string r;
    for (auto & l : Serial.out) if (! l.compare (0, 6, "- ovr ")) { if (! r.empty ()) return "MEHRFACH"; r = l; }
    std::string a;
    for (auto & l : logring) if (! l.compare (0, 6, "- ovr ")) a = l;
    if (r != a) return "MITSCHNITT/API UNGLEICH: \"" + r + "\" / \"" + a + "\"";
    return r;
}
static void erwarte_zeile (const char * f, const std::string & soll)
{
    faelle++;
    std::string ist = ovrzeile ();
    if (ist != soll) fehl (f, "erwartet \"" + soll + "\", bekam \"" + ist + "\"");
}
static std::string heap (void) { for (auto & l : Serial.out) if (! l.compare (0, 7, "- heap ")) return l; return ""; }
int main (void)
{
    printf ("ovrzeit (%s)\n", BEZEICHNUNG);
    /* 1. Ueberlauf waehrend setup(): erster Durchlauf bei 7123 ms. Heap-Zeile muss ovr=1 zeigen (Reihenfolge),
     *    die Ueberlaufzeile kommt sofort, nicht erst nach 60 s. */
    durchlauf (7123, true);
    faelle++;
    if (heap () != "- heap free=1000 max=900 ovr=1") fehl ("erste Heap-Zeile zeigt den aktuellen Stand", "bekam \"" + heap () + "\"");
    faelle++;
    if (order_bad) fehl ("Heap-Zeile nach der Ueberlaufabfrage", "Heap-Zeile vor hasOverrun() ausgegeben");
    erwarte_zeile ("erste Zeile sofort", "- ovr #1 t=7123");
    /* 2. Drosselung: innerhalb von 60 s keine Zeile, Zaehler laeuft weiter */
    durchlauf (7200, true);   erwarte_zeile ("gedrosselt nach 77 ms", "");
    durchlauf (30000, true);  erwarte_zeile ("gedrosselt nach 23 s", "");
    durchlauf (67122, true);  erwarte_zeile ("gedrosselt bei 59999 ms", "");
    durchlauf (67123, false); erwarte_zeile ("ohne Ueberlauf keine Zeile", "");
    durchlauf (67124, true);  erwarte_zeile ("nach 60 s wieder, mit laufendem Zaehler", "- ovr #5 t=67124");
    durchlauf (67125, true);  erwarte_zeile ("gleich danach wieder gedrosselt", "");
    /* 3. Heap-Zeile eine Minute spaeter zeigt den aktuellen Stand samt diesem Durchlauf */
    durchlauf (127125, true);
    faelle++;
    if (heap () != "- heap free=1000 max=900 ovr=7") fehl ("Heap-Zeile im Betrieb", "bekam \"" + heap () + "\"");
    erwarte_zeile ("Zeile im selben Durchlauf wie die Heap-Zeile", "- ovr #7 t=127125");
    faelle++;
    if (order_bad) fehl ("Reihenfolge im Betrieb", "Heap-Zeile vor hasOverrun() ausgegeben");
    /* 4. millis()-Umlauf: Differenzbildung, keine Zeile 32 ms nach der letzten */
    durchlauf (0xFFFFFFF0UL, true); erwarte_zeile ("vor dem Umlauf", "- ovr #8 t=4294967280");
    durchlauf (0x10UL, true);       erwarte_zeile ("nach dem Umlauf gedrosselt", "");
    durchlauf (100000UL, true);     erwarte_zeile ("nach dem Umlauf und 60 s wieder", "- ovr #10 t=100000");
    /* 5. Laengste Form passt ungekuerzt */
    esp_uart_overruns = 0xFFFFFFFEUL;
    durchlauf (0xFFFFFFFFUL, true); erwarte_zeile ("laengste Form", "- ovr #4294967295 t=4294967295");
    printf ("  Faelle: %d\n", faelle);
    printf ("%s  ovrzeit %s: %d Fehler\n", fails ? "FEHL" : "OK  ", BEZEICHNUNG, fails);
    return fails ? 1 : 0;
}
