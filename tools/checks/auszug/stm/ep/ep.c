/* Pruefstand EP (Paket 2026-10-10, A55, AKP.1 und AKP.2): ESP-Neustart bei laufendem STM.
 *
 * Laesst den ECHTEN Code laufen: esp8266_get_message() aus src/esp8266/esp8266.c und var_send_buf()
 * samt Nachsendeliste aus src/vars/vars.c (dieselben Schnitte wie W2), ausgeschnitten von extract.py.
 * Nachgebildet sind die UART, die Uhr (uptime = sim_ms / 1000), der ESP und von main.c genau drei
 * Dinge, die extract.py am Original statisch prueft:
 *   - schedule_esp8266_messages(): IPADDRESS und SYNCVARS merken den Vollabgleich vor (var_sync_pending)
 *   - der Vollabgleich laeuft ohne is_online-Bedingung
 *   - die periodischen Sendungen (N15 RTC-Temperatur, N31, N17 DS18xx, T00 Uhrzeit) haengen an
 *     esp8266.is_online
 * Der ESP: lebt (quittiert nach 20 ms, mit "ACK xy", sobald er CAP gemeldet hat) oder bootet. Im
 * Reset selbst (ESP_RESET_MS) gehen Zeilen verloren; danach haelt sein 256-Byte-Empfangspuffer sie,
 * bis setup() endet, und er quittiert sie dann der Reihe nach -- fuer den wartenden STM zu spaet. Er sendet CAP/FIRMWARE/IPADDRESS je Fall wie setup() und wifi.cpp, und
 * er fordert wie var_sync_check() hoechstens dreimal je 10 s SYNCVARS nach, solange der
 * Vollabgleich nicht angekommen ist -- erst ab der eigenen IP.
 * Zeilenverlust: je Fall einzeln abschaltbare ESP-Zeilen.
 *
 * Zieltypen (L256): pruefstand.h; mit -DPRUEFSTAND_SCHNELLE_TYPEN auch uint_fast8_t/16_t auf 32 Bit.
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pruefstand.h"
#include "esp8266.h"
#include "vars.h"
#include "weather.h"

/* ---------- Umgebung des Ziels ---------- */
volatile uint32_t   uptime;
static uint32_t     sim_ms;
ESP8266_GLOBALS     esp8266;
WEATHER_GLOBALS     weather;
uint_fast8_t        var_send_busy;
static unsigned     wd_reloads;
void watchdog_reload (void) { wd_reloads++; }
static void log_flush (void) {}
static void log_puts (const char * s) { (void) s; }
static int  log_printf (const char * fmt, ...) { (void) fmt; return 0; }
#define debug_log_printf(...)
#define debug_log_flush()
#define debug_log_message(s)

/* ---------- ESP-Nachbildung ---------- */
#define MAXQ 512
typedef struct { uint32_t due; char text[96]; } RX;
static RX       rxq[MAXQ];
static int      rx_tail, rx_cur = -1, rx_pos;
static uint32_t esp_alive_from;     /* ab hier quittiert der ESP (Ende von setup()) */
static uint32_t esp_dead_from;      /* ab hier bootet er (Neustart) */
static int      esp_synced;         /* Vollabgleich beim ESP angekommen (Abschlusszeile N01ff) */
static int      lines_lost_dead;    /* var-Zeilen, die im Reset selbst verloren gehen (vor Serial.begin) */
static int      buffered, buf_bytes;/* waehrend setup() im 256-Byte-Empfangspuffer des ESP gehalten */
#define ESP_RESET_MS    100         /* Reset und ROM-Lader: hier gesendete Zeichen sind weg */
#define ESP_RXBUF       256         /* HardwareSerial-Empfangspuffer des ESP8266 */
static int      sync_lines;         /* Zeilen des Vollabgleichs, die der ESP quittiert hat */

static int esp_alive (void) { return ! (sim_ms >= esp_dead_from && sim_ms < esp_alive_from); }

static void
rx_push (uint32_t due, const char * text)
{
    RX * r;
    if (rx_tail >= MAXQ) { fprintf (stderr, "rxq voll\n"); exit (2); }
    r = &rxq[rx_tail++];
    r->due = due; snprintf (r->text, sizeof (r->text), "%s", text);
}

static char     txline[256];
static int      txlen;
void esp8266_uart_puts (const char * s)
{
    for (; *s; s++)
    {
        txline[txlen++] = *s;
        if (*s == '\n')
        {
            txline[txlen - 2] = '\0'; txlen = 0;
            if (! strncmp (txline, "var ", 4))
            {
                const char * star = strrchr (txline, '*');
                char         ack[16];
                uint32_t     due = sim_ms + 20;
                if (! esp_alive ())
                {
                    /* Im Reset verloren; danach haelt der Empfangspuffer die Zeile, bis loop() laeuft. */
                    if (sim_ms < esp_dead_from + ESP_RESET_MS || buf_bytes + (int) strlen (txline) + 2 > ESP_RXBUF)
                    {
                        lines_lost_dead++; continue;
                    }
                    buf_bytes += (int) strlen (txline) + 2;
                    due = esp_alive_from + 20 * (uint32_t) ++buffered;
                }
                if (! strncmp (txline + 4, "N01", 3)) { sync_lines++; if (! strncmp (txline + 4, "N01ff", 5)) esp_synced = 1; }
                if (star && strlen (star) == 5)
                {
                    snprintf (ack, sizeof (ack), "ACK %02x\r\n", (unsigned) ((strtol (star + 1, 0, 16) >> 8) & 0xFF));
                    rx_push (due, ack);
                }
                rx_push (due, ".\r\n");
            }
        }
    }
}
void esp8266_uart_putc (uint_fast8_t c) { char b[2] = { (char) c, 0 }; esp8266_uart_puts (b); }
void esp8266_uart_flush (void) {}
void esp8266_send_cmd (const char * cmd, const char * args, uint_fast8_t do_log) { (void) cmd; (void) args; (void) do_log; }

static int
rx_next (void)
{
    int i, best = -1;
    for (i = 0; i < rx_tail; i++)
        if (rxq[i].text[0] && rxq[i].due <= sim_ms && (best < 0 || rxq[i].due < rxq[best].due)) best = i;
    return best;
}
uint_fast8_t esp8266_uart_char_available (void) { return (rx_cur >= 0 || rx_next () >= 0) ? 1 : 0; }
uint_fast8_t esp8266_uart_poll (uint_fast8_t * ch)
{
    if (rx_cur < 0) { rx_cur = rx_next (); rx_pos = 0; if (rx_cur < 0) return 0; }
    *ch = (uint_fast8_t) rxq[rx_cur].text[rx_pos++];
    if (! rxq[rx_cur].text[rx_pos]) { rxq[rx_cur].text[0] = '\0'; rx_cur = -1; }
    return 1;
}

/* ---------- der echte Code ---------- */
uint_fast8_t schedule_esp8266_messages (void);
#include SNIPPET

/* ---------- main.c-Nachbildung (statisch gegengeprueft, siehe extract.py) ---------- */
static int      var_sync_pending;
static int      online_low_seen;    /* is_online war nach dem Neustart einmal 0 */
static uint32_t online_again_at;    /* erster Zeitpunkt mit is_online = 1 nach dem Tiefpunkt */

uint_fast8_t
schedule_esp8266_messages (void)
{
    uint_fast8_t r = esp8266_get_message ();
#ifdef HAVE_A2
    var_weather_query_end (r);
#endif
    if (r == ESP8266_IPADDRESS || r == ESP8266_SYNCVARS) var_sync_pending = 1;      /* main.c, beide Zweige */
    if (r == ESP8266_OK || r == ESP8266_NAK) var_send_busy = 0;
    if (! esp8266.is_online) online_low_seen = 1;
    else if (online_low_seen && ! online_again_at) online_again_at = sim_ms;
    sim_ms++;
    uptime = sim_ms / 1000;
    return r;
}

static void
send (const char * p)
{
    char b[32];
    strcpy (b, p);
    var_send_reload_budget_reset ();
    (void) var_send_buf (b, 3);
}

/* var_send_all_variables(), verkuerzt: zehn Zeilen, die letzte ist die Abschlusszeile. */
static void
full_sync (void)
{
    static const char * z[] = { "N010001", "N010102", "N010203", "N010304", "N010405", "N010506", "N010607", "N010708", "N010809", "N01ff00" };
    int i;
    for (i = 0; i < 10; i++) send (z[i]);
}

/* ---------- Faelle ---------- */
#define L_CAP       0x01            /* Zeilenverlust je ESP-Zeile */
#define L_IP        0x02
#define L_SV1       0x04
#define L_SV2       0x08
#define L_SV3       0x10
#define L_CAPBAD    0x20            /* CAP verstuemmelt ("CAP var-cr") */

typedef enum { ESP_SPEC, ESP_CLIENT, ESP_AP, ESP_SPAET, ESP_OHNE_WLAN } ESPART;
static const char * const espart_name[] =
{
    "Spec: 1 s still, CAP, FIRMWARE, 9 s still, SYNCVARS",
    "Client: CAP, WLAN in setup() nach 9 s, IPADDRESS, FIRMWARE",
    "AP-Modus: CAP, OK ap, IPADDRESS 192.168.4.1, FIRMWARE nach rund 2 s",
    "WLAN erst nach setup(): CAP, 15 s Verbindungsversuch, FIRMWARE, IPADDRESS bei +20 s",
    "kein WLAN: CAP, 15 s Verbindungsversuch, FIRMWARE, nie IPADDRESS",
};

typedef struct
{
    int             timeouts;       /* Zeitueberschreitungen im Neustartfenster (bis +20 s) */
    int             timeouts_spaet; /* danach */
    int             sent_late;      /* periodische Sendungen nach dem Fenster */
    int             online_end;
    int             low_seen;
    uint32_t        again_at;
    int             synced;
    int             syncvars_sent;
} ERG;

static void
reset_sim (void)
{
    memset (rxq, 0, sizeof (rxq)); rx_tail = 0; rx_cur = -1; txlen = 0;
    memset (var_retry_slots, 0, sizeof (var_retry_slots));
    var_send_timeout_cnt = var_send_nested_cnt = 0;
    esp8266.cap_var_crc = 1; esp8266.is_online = 1;                 /* laufender Betrieb: Bruecke steht, CAP festgeschrieben */
    wd_reloads = 0; esp8266_ack_drops = 0;
    var_sync_pending = 0; online_low_seen = 0; online_again_at = 0;
    esp_synced = 1; lines_lost_dead = 0; sync_lines = 0; buffered = 0; buf_bytes = 0;
    sim_ms = ((sim_ms / 1000) + 60) * 1000;
    uptime = sim_ms / 1000;
    esp_dead_from = 0xFFFFFFFFu; esp_alive_from = 0;
#ifdef HAVE_A2
    var_weather_busy = 0;
#endif
}

/* Ein Lauf: ESP startet bei t0 neu; periodische Sendungen an den Versaetzen per[] (ms ab t0), dazu
 * zwei spaete bei +100 s und +110 s; die Simulation laeuft bis +120 s. */
static ERG
lauf (ESPART art, int verlust, const int * per, int nper, int stm_start)
{
    ERG         g;
    uint32_t    t0, ip_at = 0, ende;
    int         done[8] = { 0 }, i, tries = 0, timeouts_window = -1;
    uint32_t    next_check = 0;
    int         late[2] = { 100000, 110000 }, late_done[2] = { 0, 0 };
    char        ip_line[48];

    memset (&g, 0, sizeof (g));
    reset_sim ();
    t0 = sim_ms;
    if (stm_start) { esp8266.is_online = 0; esp8266.cap_var_crc = 0; }  /* STM-Start: Vorbelegung, Bruecke unbekannt */
    esp_dead_from = t0; esp_synced = 0;                                  /* ESP-Variablen auf Vorgabe (vars_init) */

    if (! (verlust & L_CAP)) rx_push (t0 + 1000, (verlust & L_CAPBAD) ? "CAP var-cr\r\n" : "CAP var-crc\r\n");
    switch (art)
    {
        case ESP_SPEC:
            rx_push (t0 + 1100, "FIRMWARE 3.2.30\r\n");
            esp_alive_from = t0 + 10000;
            ip_at = t0;                                 /* die Spec-Folge hat keine IPADDRESS-Zeile; die Uhr laeuft ab t0 */
            break;
        case ESP_CLIENT:
            rx_push (t0 + 9000, "AP Heimnetz\r\n"); rx_push (t0 + 9000, "MODE client\r\n");
            ip_at = t0 + 9000; rx_push (t0 + 9300, "FIRMWARE 3.2.30\r\n");
            esp_alive_from = t0 + 9500;
            break;
        case ESP_AP:
            rx_push (t0 + 1500, "OK ap\r\n"); rx_push (t0 + 1500, "AP wordclock\r\n"); rx_push (t0 + 1500, "MODE ap\r\n");
            ip_at = t0 + 1500; rx_push (t0 + 1700, "FIRMWARE 3.2.30\r\n");
            esp_alive_from = t0 + 2000;
            break;
        case ESP_SPAET:
            rx_push (t0 + 16000, "FIRMWARE 3.2.30\r\n");
            esp_alive_from = t0 + 16000;
            rx_push (t0 + 20000, "AP Heimnetz\r\n"); rx_push (t0 + 20000, "MODE client\r\n");
            ip_at = t0 + 20000;
            break;
        case ESP_OHNE_WLAN:
            rx_push (t0 + 16000, "FIRMWARE 3.2.30\r\n");
            esp_alive_from = t0 + 16000;
            ip_at = 0;
            break;
    }
    snprintf (ip_line, sizeof (ip_line), "IPADDRESS %s\r\n", art == ESP_AP ? "192.168.4.1" : "192.168.1.50");
    if (ip_at && art != ESP_SPEC && ! (verlust & L_IP)) rx_push (ip_at, ip_line);
    if (ip_at) next_check = ip_at + 10000;

    ende = t0 + 120000;
    while (sim_ms < ende)
    {
        var_send_reload_budget_reset ();
        schedule_esp8266_messages ();

        if (var_sync_pending) { var_sync_pending = 0; full_sync (); }

        for (i = 0; i < nper; i++)
            if (! done[i] && sim_ms >= t0 + (uint32_t) per[i])
            {
                static const char * id[4] = { "N150020", "N310001", "N170030", "T000000" };
                done[i] = 1;
                if (esp8266.is_online) send (id[i % 4]);
            }
        for (i = 0; i < 2; i++)
            if (! late_done[i] && sim_ms >= t0 + (uint32_t) late[i])
            {
                late_done[i] = 1;
                if (esp8266.is_online) { int v0 = var_send_timeout_cnt; send ("N150021"); g.sent_late++; g.timeouts_spaet += var_send_timeout_cnt - v0; }
            }

        /* var_sync_check() des ESP: erst ab eigener IP, je 10 s, hoechstens dreimal */
        if (next_check && sim_ms >= next_check && esp_alive ())
        {
            next_check = sim_ms + 10000;
            if (! esp_synced && tries < 3)
            {
                int lost = (tries == 0 && (verlust & L_SV1)) || (tries == 1 && (verlust & L_SV2)) || (tries == 2 && (verlust & L_SV3));
                tries++;
                g.syncvars_sent++;
                if (! lost) rx_push (sim_ms, "SYNCVARS\r\n");
            }
        }

        if (timeouts_window < 0 && sim_ms >= t0 + 20000) timeouts_window = var_send_timeout_cnt;
    }
    g.timeouts  = timeouts_window;
    g.online_end = esp8266.is_online ? 1 : 0;
    g.low_seen  = online_low_seen;
    g.again_at  = online_again_at ? online_again_at - t0 : 0;
    g.synced    = esp_synced;
    return g;
}

static int fails, cases;
#define CHECK(name, cond) do { cases++; if (cond) printf ("  OK    %s\n", name); else { fails++; printf ("  FEHL  %s\n", name); } } while (0)

static void
zeige (const ERG * g)
{
    printf ("        Zeitueberschreitungen bis +20 s: %d, danach: %d; offline gesehen: %s; wieder online bei +%u ms; "
            "Vollabgleich angekommen: %s; SYNCVARS-Anforderungen: %d; is_online am Ende: %d\n",
            g->timeouts, g->timeouts_spaet, g->low_seen ? "ja" : "nein", (unsigned) g->again_at,
            g->synced ? "ja" : "nein", g->syncvars_sent, g->online_end);
}

int
main (void)
{
    static const int placements[][4] =
    {
        {  200, 2500, 5000, 7500 },         /* erste Sendung vor CAP: ihre Wartezeit ueberspannt CAP */
        { 1500, 3000, 5000, 7000 },         /* alle nach CAP */
        {  900,  950, 3000, 6000 },         /* zwei kurz vor CAP */
        {  -50, 2000, 4500, 8500 },         /* eine kurz vor dem Neustart: Quittung geht im Boot verloren */
    };
    char    n[240];
    int     a, p;
    ERG     g;

    printf ("[0] statisch: main.c und ESP-Code\n");
    CHECK ("main.c: IPADDRESS- und SYNCVARS-Zweig merken den Vollabgleich vor", EP_MAIN_SYNC == 1);
    CHECK ("main.c: Vollabgleich ohne is_online-Bedingung", EP_MAIN_SYNC_UNGATED == 1);
    snprintf (n, sizeof (n), "main.c: %d Aufrufstellen der periodischen Sendungen, %d an is_online, %d Antwort auf ESP-Zeile",
              EP_MAIN_SITES, EP_MAIN_GATED, EP_MAIN_REPLY);
    CHECK (n, EP_MAIN_SITES >= 5 && EP_MAIN_GATED + EP_MAIN_REPLY == EP_MAIN_SITES && EP_MAIN_REPLY <= 1);
    CHECK ("ESP setup(): CAP vor wifi_ap()/wifi_connect() und vor FIRMWARE (Reihenfolge CAP vor IPADDRESS)", EP_ESP_CAP_FIRST == 1);
    snprintf (n, sizeof (n), "ESP wifi.cpp: %d Stellen senden IPADDRESS (Client in setup, AP, Client spaet, AP spaet)", EP_ESP_IP_SITES);
    CHECK (n, EP_ESP_IP_SITES == 4);
    CHECK ("ESP var_sync_check(): SYNCVARS erst nach eigener IP", EP_ESP_SYNC_AFTER_IP == 1);

    for (a = ESP_SPEC; a <= ESP_SPAET; a++)
        for (p = 0; p < 4; p++)
        {
            printf ("[1] AKP.1  %s; Sendungen bei %d/%d/%d/%d ms\n", espart_name[a],
                    placements[p][0], placements[p][1], placements[p][2], placements[p][3]);
            g = lauf ((ESPART) a, 0, placements[p], 4, 0); zeige (&g);
            snprintf (n, sizeof (n), "hoechstens eine Zeitueberschreitung im Neustartfenster (%d)", g.timeouts);
            CHECK (n, g.timeouts <= 1);
            CHECK ("  offline ab CAP, danach wieder online", g.low_seen && g.online_end);
            CHECK ("  Vollabgleich angekommen, spaete Sendungen ohne Zeitueberschreitung", g.synced && g.sent_late == 2 && g.timeouts_spaet == 0);
        }

    printf ("[2] AKP.2  Zeilenverlust\n");
    {
        static const int per[4] = { 1500, 3000, 5000, 7000 };
        struct { const char * name; ESPART art; int verlust; int erwartet; } v[] =
        {
            /* erwartet: 0 = wie heute (nie offline, kein Fortschritt durch A55), 1 = offline, wieder online,
             *           2 = offline bis zum naechsten WLAN-Ereignis (Befund, siehe Bericht) */
            { "CAP verloren -- Verhalten wie heute",                         ESP_CLIENT, L_CAP,    0 },
            { "CAP verstuemmelt (\"CAP var-cr\") -- wie heute",               ESP_CLIENT, L_CAPBAD, 0 },
            { "IPADDRESS verloren -- SYNCVARS nach 10 s heilt",              ESP_CLIENT, L_IP,     1 },
            { "IPADDRESS und erstes SYNCVARS verloren -- zweites heilt",    ESP_CLIENT, L_IP | L_SV1, 1 },
            { "IPADDRESS und zwei SYNCVARS verloren -- drittes heilt",      ESP_CLIENT, L_IP | L_SV1 | L_SV2, 1 },
            { "AP-Modus, IPADDRESS verloren -- SYNCVARS heilt",              ESP_AP,     L_IP,     1 },
            { "IPADDRESS und alle drei SYNCVARS verloren",                  ESP_CLIENT, L_IP | L_SV1 | L_SV2 | L_SV3, 2 },
        };
        int i;
        for (i = 0; i < (int) (sizeof (v) / sizeof (v[0])); i++)
        {
            printf ("[2] %s\n", v[i].name);
            g = lauf (v[i].art, v[i].verlust, per, 4, 0); zeige (&g);
            if (v[i].erwartet == 0)
            {
                CHECK ("  nie offline, am Ende online (A55 greift nicht, Rueckfall auf heute)", ! g.low_seen && g.online_end);
            }
            else if (v[i].erwartet == 1)
            {
                snprintf (n, sizeof (n), "  hoechstens eine Zeitueberschreitung (%d), wieder online, Vollabgleich angekommen", g.timeouts);
                CHECK (n, g.timeouts <= 1 && g.online_end && g.synced && g.timeouts_spaet == 0);
            }
            else
            {
                CHECK ("  BEFUND: bleibt offline bis zur naechsten IPADDRESS-Zeile, keine Sendung in die Leere", ! g.online_end && g.timeouts <= 1 && g.timeouts_spaet == 0);
            }
        }
    }

    printf ("[3] Grenzfaelle\n");
    {
        static const int per[4] = { 1500, 3000, 5000, 7000 };
        printf ("[3] kein WLAN nach dem Neustart\n");
        g = lauf (ESP_OHNE_WLAN, 0, per, 4, 0); zeige (&g);
        CHECK ("  offline, keine Zeitueberschreitung -- wie nach einem Kaltstart ohne WLAN", ! g.online_end && g.timeouts <= 1 && g.timeouts_spaet == 0);
        printf ("[3] Start des STM (is_online vorbelegt 0), CAP kommt beim Zeilenparser an\n");
        g = lauf (ESP_CLIENT, 0, per, 4, 1); zeige (&g);
        CHECK ("  keine Zeitueberschreitung, online ab IPADDRESS, Vollabgleich angekommen", g.timeouts == 0 && g.online_end && g.synced);
    }

    printf ("\n%d Faelle, %d fehlgeschlagen\n", cases, fails);
    return fails ? 1 : 0;
}
