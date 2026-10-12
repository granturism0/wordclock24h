/* Pruefstand A67 (Paket 2026-10-11, S3.10): OTA-Ankuendigung "OTA start" vom ESP.
 * Baut auf dem Pruefstand EP (A55) auf: dessen extract.py schneidet den ECHTEN Zeilenparser aus esp8266.c
 * (samt Pruefsummen- und Quittungslogik), dessen ep.c liefert UART, Uhr und Zeilenwarteschlange (rx_push).
 * ep.c wird mit umbenanntem main eingebunden; geprueft wird nur der Parser. */
#define main ep_main
#include EP_C
#undef main

static int a67_faelle, a67_fehl; static char a67_was[200];
static void a67_pruef (int ok, const char * w) { a67_faelle++; if (! ok) { a67_fehl++; printf ("  FEHL  %s\n", w); } }
static uint_fast8_t zeile (const char * text)
{
    char b[96]; uint_fast8_t r = ESP8266_TIMEOUT; int k;
    snprintf (b, sizeof b, "%s\r\n", text);
    rx_push (sim_ms, b);
    for (k = 0; k < 200 && (r = esp8266_get_message ()) == ESP8266_TIMEOUT; k++) sim_ms++;
    return r;
}

int main (void)
{
    static const char * fast[] = { "OTA start x", "OTA star", "OTA Start", "OTA start ", "xOTA start", "OTA", "OTA  start", "OTA-start", "OTAstart" };
    int i, cap, crc;
    uint_fast8_t r;

    for (cap = 0; cap < 2; cap++) for (crc = 0; crc < 2; crc++)
    {
        esp8266.is_online = 1; esp8266_cap_var_crc_seen = (uint_fast8_t) cap; esp8266.cap_var_crc = (uint_fast8_t) crc;
        r = zeile ("OTA start");
        snprintf (a67_was, sizeof a67_was, "OTA start (cap_seen %d, cap_var_crc %d): r=%u online=%u cap_seen=%u cap_var_crc=%u",
                  cap, crc, (unsigned) r, (unsigned) esp8266.is_online, (unsigned) esp8266_cap_var_crc_seen, (unsigned) esp8266.cap_var_crc);
        a67_pruef (r == ESP8266_STATUS && esp8266.is_online == 0 && esp8266_cap_var_crc_seen == cap && esp8266.cap_var_crc == crc, a67_was);

        r = zeile ("IPADDRESS 192.168.1.5");
        a67_pruef (r == ESP8266_IPADDRESS && esp8266.is_online == 1, "nach OTA start: IPADDRESS setzt nicht wieder online");

        r = zeile ("OTA start");
        r = zeile ("SYNCVARS");
        a67_pruef (r == ESP8266_SYNCVARS && esp8266.is_online == 1, "nach OTA start: SYNCVARS (OTA gescheitert) setzt nicht wieder online");
    }
    for (i = 0; i < (int) (sizeof fast / sizeof fast[0]); i++)
    {
        esp8266.is_online = 1; esp8266_cap_var_crc_seen = 0;
        r = zeile (fast[i]);
        snprintf (a67_was, sizeof a67_was, "Fast-Treffer \"%s\": r=%u online=%u (Soll: kein STATUS, online bleibt 1)", fast[i], (unsigned) r, (unsigned) esp8266.is_online);
        a67_pruef (r != ESP8266_STATUS && esp8266.is_online == 1, a67_was);
    }
    /* CAP verhaelt sich wie bisher (A55): offline UND gemerkt -- die neue Zeile hat daran nichts geaendert */
    esp8266.is_online = 1; esp8266_cap_var_crc_seen = 0;
    r = zeile ("CAP var-crc");
    a67_pruef (r == ESP8266_STATUS && esp8266.is_online == 0 && esp8266_cap_var_crc_seen == 1, "CAP var-crc: Verhalten veraendert");
    printf ("a67: %d Pruefungen, %d FEHL -- %s\n", a67_faelle, a67_fehl, a67_fehl ? "FEHL" : "OK");
    return a67_fehl ? 1 : 0;
}
