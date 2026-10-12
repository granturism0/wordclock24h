/* Teil 1 von otarueck: Zeitlogik von http_ota_restart_poll () am echten Code. */
#include <stdint.h>
#include <stdio.h>
#include <string>
#define uint_fast8_t unsigned int
#define F(s) (s)
static uint32_t fake_ms = 1000;
static uint32_t millis (void) { return fake_ms; }
static int restarts = 0;
struct { void restart (void) { restarts++; } } ESP;
struct { std::string out; void println (const char * s) { out += s; out += "\r\n"; } void flush (void) {} } Serial;
#include "rueck_code.inc"
static int fails = 0, faelle = 0;
static void pruef (bool ok, const char * f) { faelle++; if (! ok) { fails++; printf ("      FEHLER: %s\n", f); } }
#ifndef RUECK_FEHLT
static void vormerken (void) { ota_restart_pending = 1; ota_restart_since = millis (); }   /* wie im Erfolgszweig */
static void tick (uint32_t ms) { for (uint32_t t = 0; t < ms; t += 100) { fake_ms += 100; http_ota_restart_poll (); } }
#endif
int main (void)
{
    printf ("otarueck dynamisch (%s)\n", BEZEICHNUNG);
#ifdef RUECK_FEHLT
    pruef (false, "http_ota_restart_poll () fehlt -- kein Rueckfall-Neustart");
#else
    tick (60000);                         pruef (restarts == 0, "ohne Vormerkung kein Neustart (60 s)");
    vormerken (); fake_ms += 9999; http_ota_restart_poll ();
                                          pruef (restarts == 0, "kein Neustart nach 9'999 ms");
    fake_ms += 1; http_ota_restart_poll ();
                                          pruef (restarts == 1, "Neustart nach 10'000 ms");
    pruef (Serial.out == "- ota: Rueckfall-Neustart\r\n", "Zeile \"- ota: Rueckfall-Neustart\" im Mitschnitt");
    tick (60000);                         pruef (restarts == 1, "genau ein Neustart, nicht wiederholt");
    vormerken (); fake_ms += 3000; ota_restart_pending = 0;   /* wie /api/local_esp_restart */
    tick (60000);                         pruef (restarts == 1, "nach /api/local_esp_restart kein zweiter Neustart");
    fake_ms = 0xFFFFFFFFu - 4000u; vormerken ();
    fake_ms += 9000; http_ota_restart_poll ();            pruef (restarts == 1, "Wickeln: 9 s ueber die Grenze noch kein Neustart");
    fake_ms += 1000; http_ota_restart_poll ();            pruef (restarts == 2, "Wickeln: nach 10 s ueber die Grenze Neustart");
#endif
    printf ("  Faelle: %d, %s\n", faelle, fails ? "FEHL" : "OK");
    return fails ? 1 : 0;
}
