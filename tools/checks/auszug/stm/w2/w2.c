/* Pruefstand W2 (Paket 2026-10-09, A2, AKW.1 und AKW.2): Warten auf die Wetterantwort.
 *
 * Laesst den ECHTEN Firmware-Code laufen: esp8266_get_message() aus src/esp8266/esp8266.c,
 * var_send_buf() samt A2-Zustand aus src/vars/vars.c (der Abschnitt, den auch AKS.5 einbindet) und
 * weather_query() aus src/weather/weather.c -- ausgeschnitten von extract.py.
 * Nachgebildet sind nur die UART, die Uhr (uptime = sim_ms / 1000, wie im Zeitgeber-Interrupt)
 * und der ESP. schedule_esp8266_messages() bildet main.c nach; dass main.c dort wirklich
 * var_weather_query_end() ruft, prueft das Auszugsskript statisch (MAIN_HOOK, eigener Fall).
 *
 * Zieltypen (L256): pruefstand.h; mit -DPRUEFSTAND_SCHNELLE_TYPEN auch uint_fast8_t/16_t auf 32 Bit.
 * HAVE_A2 setzt das Laufskript, wenn der gepruefte Stand A2 kennt -- ohne A2 laesst sich der
 * Ausgangsstand sonst gar nicht uebersetzen, und die Gegenprobe zeigte "kaputt" statt "schlaegt an".
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
#define MAXQ 256
typedef struct { uint32_t due; char text[96]; } RX;
static RX       rxq[MAXQ];
static int      rx_tail, rx_cur = -1, rx_pos;

static int32_t  ack_delay;          /* ms bis zum Punkt fuer eine var-Zeile; < 0: der ESP schweigt */
static uint32_t ack_at;             /* > 0: absoluter Zeitpunkt des Punkts, ueberschreibt ack_delay */
static int      var_lines;          /* empfangene var-Zeilen */
static int      weather_cmds;       /* empfangene Wetter-Kommandos */
static char     last_cmd[16];

static void
rx_push (uint32_t due, const char * text)
{
    RX * r = &rxq[rx_tail++];
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
                const char *    star = strrchr (txline, '*');
                char            ack[16];
                uint32_t        due = ack_at ? ack_at : sim_ms + (uint32_t) ack_delay;

                var_lines++;
                if (ack_at || ack_delay >= 0)
                {
                    /* Wie der ESP: mit Marke zuerst "ACK xy" (hoeheres Byte der EMPFANGENEN Marke), dann der Punkt. */
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
}
void esp8266_uart_putc (uint_fast8_t c) { char b[2] = { (char) c, 0 }; esp8266_uart_puts (b); }
void esp8266_uart_flush (void) {}

/* weather_query() sendet ueber esp8266_send_cmd(); hier wird nur mitgezaehlt. */
void esp8266_send_cmd (const char * cmd, const char * args, uint_fast8_t do_log)
{
    (void) args; (void) do_log;
    snprintf (last_cmd, sizeof (last_cmd), "%s", cmd);
    weather_cmds++;
}

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

/* ---------- main.c-Nachbildung ---------- */
static int      nested_query_at;    /* > 0: verschachtelter Anstoss (RPC "get weather") zu diesem Zeitpunkt */
static int      nested_spam;        /* 1: verschachtelter Anstoss bei JEDER Gelegenheit */

uint_fast8_t
schedule_esp8266_messages (void)
{
    uint_fast8_t r = esp8266_get_message ();
#ifdef HAVE_A2
    var_weather_query_end (r);                                  /* main.c, direkt nach esp8266_get_message() */
#endif
    if (var_send_cur_buf && ((nested_query_at && sim_ms >= (uint32_t) nested_query_at) || nested_spam))
    {
        nested_query_at = 0;
        weather_query (WEATHER_QUERY_ID_TEXT);                  /* wie schedule_esp8266_cmd() bei GET_WEATHER_RPC_VAR */
    }
    if (r == ESP8266_OK || r == ESP8266_NAK) var_send_busy = 0;
    sim_ms++;
    uptime = sim_ms / 1000;
    return r;
}

static void
main_loop (uint32_t ms)
{
    uint32_t end = sim_ms + ms;
    while (sim_ms < end) { var_send_reload_budget_reset (); schedule_esp8266_messages (); }
}

static uint32_t     dauer;          /* ms, die der letzte send() in var_send_buf() stand */
static int          angenommen;     /* Rueckgabe des letzten send() */
static uint32_t     normal;         /* ms bis zum dritten Sekundenschritt ab dem Start des letzten send(): die normale Wartezeit */

static void
send (const char * p)
{
    char b[32]; uint32_t t0 = sim_ms;
    normal = ((t0 / 1000) + VAR_SEND_TIMEOUT_SEC) * 1000 - t0;
    strcpy (b, p);
    var_send_reload_budget_reset ();
    angenommen = var_send_buf (b, 3);
    dauer = sim_ms - t0;
}

/* Neuer Fall: Uhr auf die naechste Sekundengrenze + offset_ms, ESP leer, Wetterort gesetzt. */
static void
reset (uint32_t offset_ms)
{
    memset (rxq, 0, sizeof (rxq)); rx_tail = 0; rx_cur = -1; txlen = 0;
    memset (var_retry_slots, 0, sizeof (var_retry_slots));
    var_send_timeout_cnt = var_send_nested_cnt = 0;
    esp8266.cap_var_crc = 0;
    ack_delay = -1; ack_at = 0; var_lines = 0; weather_cmds = 0; wd_reloads = 0; esp8266_ack_drops = 0;
    nested_query_at = 0; nested_spam = 0;
    strcpy (weather.appid, "0123456789abcdef");
    strcpy (weather.city, "Zuerich");
    strcpy (weather.lon, "8.54");
    strcpy (weather.lat, "47.37");
    sim_ms = ((sim_ms / 1000) + 30) * 1000 + offset_ms;      /* 30 s Abstand: jeder fruehere Abruf ist sicher abgelaufen */
    uptime = sim_ms / 1000;
#ifdef HAVE_A2
    var_weather_busy = 0;
#endif
}

static int fails, cases;
static unsigned drops (void) { return esp8266_ack_drops; }     /* verworfene Quittungen: Zuordnung passte nicht */
#define CHECK(name, cond) do { cases++; if (cond) printf ("  OK    %s\n", name); else { fails++; printf ("  FEHL  %s\n", name); } } while (0)

int
main (void)
{
    static const char * endzeilen[] = { "WEATHER Zuerich 12 Grad\r\n", "WEATHER_FC Zuerich morgen 9 Grad\r\n",
                                        "WICON 02d\r\n", "WICON_FC 10n\r\n", "ERROR weather leer\r\n" };
    static const char * messzeilen[] = { "- weather fc=0 ms=4990 ok\r\n", "- weather fc=1 ms=4990 ok\r\n",
                                         "- weather fc=0 ms=4990 ok\r\n", "- weather fc=1 ms=4990 ok\r\n", "- weather fc=0 ms=4990 leer\r\n" };
    char        n[160];
    uint32_t    t0;
    int         i, cap;

    printf ("[0] statisch: main.c reicht jede Nachricht an var_weather_query_end() weiter\n");
    CHECK ("schedule_esp8266_messages(): var_weather_query_end (msg_rtc) direkt nach esp8266_get_message()", MAIN_HOOK == 1);

    printf ("[1] AKW.1: Abruf laeuft, ESP schweigt 5 s, dann Quittung\n");
    reset (0); t0 = sim_ms;
    weather_query (WEATHER_QUERY_ID_TEXT);
    rx_push (t0 + 5000, "WEATHER Zuerich 12 Grad\r\n");
    ack_at = t0 + 5000;
    send ("N0a0001");
    snprintf (n, sizeof (n), "angenommen, kein Timeout (Dauer %u ms)", (unsigned) dauer);
    CHECK (n, angenommen == 1 && var_send_timeout_cnt == 0);
    main_loop (10000);
    CHECK ("nichts nachgesendet (eine var-Zeile)", var_lines == 1);
    CHECK ("Watchdog genau einmal nach der Quittung bedient", wd_reloads == 1);

    printf ("[2] AKW.1, Grenze: Anstoss 999 ms nach einem Sekundenschritt, Quittung nach 5'990 ms\n");
    reset (999); t0 = sim_ms;
    weather_query (WEATHER_QUERY_ID_TEXT_FC);
    ack_at = t0 + 5990;
    send ("N0a0002");
    snprintf (n, sizeof (n), "angenommen, kein Timeout (Dauer %u ms)", (unsigned) dauer);
    CHECK (n, angenommen == 1 && var_send_timeout_cnt == 0);

    printf ("[3] AKW.1: Anstoss ueber den Ort statt Koordinaten, Quittung nach 5 s\n");
    reset (0); weather.lon[0] = '\0'; t0 = sim_ms;
    weather_query (WEATHER_QUERY_ID_ICON);
    ack_at = t0 + 5000;
    send ("N0a0003");
    snprintf (n, sizeof (n), "Kommando 'wicon' gesendet und Quittung angenommen (Dauer %u ms)", (unsigned) dauer);
    CHECK (n, ! strcmp (last_cmd, "wicon") && angenommen == 1 && var_send_timeout_cnt == 0);

    printf ("[4] AKW.2: keine Endzeile, ESP schweigt -- Frist mindestens 6 s, hoechstens 7 s ab dem Anstoss\n");
    for (i = 0; i < 2; i++)
    {
        uint32_t off = i ? 999 : 0;
        reset (off); t0 = sim_ms;
        weather_query (WEATHER_QUERY_ID_TEXT);
        send ("N0b0001");
        snprintf (n, sizeof (n), "Anstoss +%u ms: aufgegeben nach %u ms, Timeout gezaehlt", (unsigned) off, (unsigned) (sim_ms - t0));
        CHECK (n, sim_ms - t0 >= 6000 && sim_ms - t0 <= 7000 && var_send_timeout_cnt == 1);
        CHECK ("  ohne Quittung kein Watchdog-Reload", wd_reloads == 0);
        send ("N0b0002");
        snprintf (n, sizeof (n), "  danach geloescht: naechstes Kommando wartet normal (%u ms)", (unsigned) dauer);
        CHECK (n, dauer == 3000);
    }

    printf ("[5] AKW.2: Frist abgelaufen, ohne dass jemand wartete -- naechstes Kommando wartet normal\n");
    reset (0);
    weather_query (WEATHER_QUERY_ID_TEXT);
    main_loop (8000);
    send ("N0b0003");
    snprintf (n, sizeof (n), "3 s wie heute (%u ms)", (unsigned) dauer);
    CHECK (n, dauer == 3000);

    printf ("[6] AKW.2: kein Abruf angestossen -- 3 s wie heute\n");
    reset (0);
    send ("N0c0001");
    snprintf (n, sizeof (n), "aufgegeben nach %u ms, Timeout gezaehlt", (unsigned) dauer);
    CHECK (n, dauer == 3000 && var_send_timeout_cnt == 1);
    reset (0); weather.appid[0] = '\0';
    weather_query (WEATHER_QUERY_ID_TEXT);
    send ("N0c0002");
    snprintf (n, sizeof (n), "ohne App-Kennung geht nichts raus und es gilt nichts als angestossen (%u ms)", (unsigned) dauer);
    CHECK (n, weather_cmds == 0 && dauer == 3000);

    /* [7] Reihenfolge wie weather.cpp:602-613 und die Hauptschleife des ESP: Endzeile, Messzeile
     * "- weather fc=.. ms=.. <ergebnis>", dann -- erst nach Rueckkehr in die Schleife -- bei Marke
     * "ACK xy" und der Punkt. Einmal ohne (cap_var_crc=0), einmal mit Pruefsumme (cap_var_crc=1). */
    for (cap = 0; cap < 2; cap++)
    {
        printf ("[7] Endzeile je Art nach 5 s, Messzeile, Quittung 30 ms danach, cap_var_crc=%d\n", cap);
        for (i = 0; i < 5; i++)
        {
            reset (0); esp8266.cap_var_crc = (uint_fast8_t) cap; t0 = sim_ms;
            weather_query (i == 2 ? WEATHER_QUERY_ID_ICON : i == 3 ? WEATHER_QUERY_ID_ICON_FC : i == 1 ? WEATHER_QUERY_ID_TEXT_FC : WEATHER_QUERY_ID_TEXT);
            rx_push (t0 + 5000, endzeilen[i]);
            rx_push (t0 + 5000, messzeilen[i]);
            ack_at = t0 + 5030;
            send ("N0d0001");
            snprintf (n, sizeof (n), "%.*s: wartendes Kommando angenommen (%u ms)", (int) strcspn (endzeilen[i], "\r"), endzeilen[i], (unsigned) dauer);
            CHECK (n, angenommen == 1 && var_send_timeout_cnt == 0 && drops () == 0);
            /* Endzeile frueh (1 s), Quittung 30 ms danach, dann schweigt der ESP: Das NAECHSTE Kommando muss
             * nach drei Sekundenschritten aufgeben (rund 2'969 ms ab Start bei +1'031; mit "ACK xy" liest die
             * Nachbildung eine Zeile mehr, dann 2'968). Wirkte der Abruf fort, wartete es bis 7 s nach dem
             * Anstoss (rund 5'969 ms). */
            reset (0); esp8266.cap_var_crc = (uint_fast8_t) cap; t0 = sim_ms;
            weather_query (WEATHER_QUERY_ID_TEXT);
            rx_push (t0 + 1000, endzeilen[i]);
            rx_push (t0 + 1000, messzeilen[i]);
            ack_at = t0 + 1030;
            send ("N0d0002");
            CHECK ("  fruehe Endzeile: wartendes Kommando angenommen", angenommen == 1 && drops () == 0);
            ack_at = 0; ack_delay = -1;
            send ("N0d0003");
            snprintf (n, sizeof (n), "  Abruf beendet: naechstes Kommando wartet normal (%u ms, erwartet %u)", (unsigned) dauer, (unsigned) normal);
            CHECK (n, angenommen == 0 && dauer == normal && dauer < 3000);
        }
        reset (0); esp8266.cap_var_crc = (uint_fast8_t) cap; t0 = sim_ms;
        weather_query (WEATHER_QUERY_ID_ICON);
        rx_push (t0 + 1000, "WEATHER Fehler 401\r\n");
        rx_push (t0 + 1000, "- weather fc=0 ms=990 fehler\r\n");
        ack_at = t0 + 1030;
        send ("N0d0004");
        ack_at = 0;
        send ("N0d0005");
        snprintf (n, sizeof (n), "Icon angefragt, WEATHER-Zeile (Fehler-cod) beendet: danach normal (%u ms, erwartet %u)", (unsigned) dauer, (unsigned) normal);
        CHECK (n, var_send_timeout_cnt == 1 && dauer == normal && dauer < 3000);
        reset (0); esp8266.cap_var_crc = (uint_fast8_t) cap; t0 = sim_ms;
        weather_query (WEATHER_QUERY_ID_TEXT);
        rx_push (t0 + 1000, "WEATHER Zuerich 12 Grad\r\n");
        rx_push (t0 + 1000, "- weather fc=0 ms=990 ok\r\n");
        send ("N0d0006");
        snprintf (n, sizeof (n), "Endzeile, danach schweigt der ESP: wartendes Kommando gibt %u ms nach dem Anstoss auf", (unsigned) (sim_ms - t0));
        CHECK (n, sim_ms - t0 >= 6000 && sim_ms - t0 <= 7000 && var_send_timeout_cnt == 1);
    }

    printf ("[8] Endzeile beendet den Abruf -- im Hauptloop, vor dem Kommando\n");
    reset (0); t0 = sim_ms;
    weather_query (WEATHER_QUERY_ID_TEXT);
    rx_push (t0 + 500, "WEATHER Zuerich 12 Grad\r\n");
    main_loop (1000);
    send ("N0e0001");
    snprintf (n, sizeof (n), "3 s wie heute (%u ms)", (unsigned) dauer);
    CHECK (n, dauer == 3000);

    printf ("[9] Endzeile ohne Anstoss aendert nichts\n");
    reset (0); t0 = sim_ms;
    rx_push (t0 + 100, "WEATHER Zuerich 12 Grad\r\n");
    rx_push (t0 + 200, "ERROR weather leer\r\n");
    main_loop (1000);
    send ("N0f0001");
    snprintf (n, sizeof (n), "3 s wie heute (%u ms)", (unsigned) dauer);
    CHECK (n, dauer == 3000);
    reset (0); ack_delay = 20;
    rx_push (sim_ms + 5, "WEATHER Zuerich 12 Grad\r\n");
    send ("N0f0002");
    CHECK ("schnelle Quittung wie heute", angenommen == 1 && dauer < 100);

    printf ("[10] zweiter Anstoss verlaengert nicht\n");
    reset (0); t0 = sim_ms;
    weather_query (WEATHER_QUERY_ID_TEXT);
    main_loop (2500);
    weather_query (WEATHER_QUERY_ID_ICON);
    send ("N0g0001");
    snprintf (n, sizeof (n), "aufgegeben %u ms nach dem ERSTEN Anstoss (ohne Verlaengerung <= 7'000)", (unsigned) (sim_ms - t0));
    CHECK (n, sim_ms - t0 >= 6000 && sim_ms - t0 <= 7000 && weather_cmds == 2);

    printf ("[11] ESP-Neustart mitten im Abruf\n");
    reset (0); t0 = sim_ms;
    weather_query (WEATHER_QUERY_ID_TEXT_FC);
    rx_push (t0 + 1000, "OK cap\r\n");
    rx_push (t0 + 1200, "FIRMWARE 3.2.28\r\n");
    rx_push (t0 + 1400, "OK time\r\n");
    send ("N0h0001");
    snprintf (n, sizeof (n), "aufgegeben %u ms nach dem Anstoss", (unsigned) (sim_ms - t0));
    CHECK (n, sim_ms - t0 >= 6000 && sim_ms - t0 <= 7000);
    send ("N0h0002");
    snprintf (n, sizeof (n), "nichts haengt: naechstes Kommando wartet normal (%u ms)", (unsigned) dauer);
    CHECK (n, dauer == 3000);

    printf ("[12] verschachtelter Anstoss waehrend des Wartens (RPC): hoechstens 7 s ab dem eigenen Start\n");
    reset (0); t0 = sim_ms;
    nested_query_at = (int) (t0 + 2500);
    send ("N0i0001");
    snprintf (n, sizeof (n), "einzelner Anstoss nach 2,5 s: aufgegeben nach %u ms", (unsigned) dauer);
    CHECK (n, weather_cmds == 1 && dauer <= 7000);
    reset (0);
    nested_spam = 1;
    send ("N0i0002");
    nested_spam = 0;
    snprintf (n, sizeof (n), "Anstoss bei jeder Gelegenheit: aufgegeben nach %u ms, Anstoesse %d", (unsigned) dauer, weather_cmds);
    CHECK (n, dauer <= 7000);
    CHECK ("  ohne Quittung kein Watchdog-Reload", wd_reloads == 0);

    printf ("[13] schnelle Quittung waehrend eines Abrufs: unveraendert\n");
    reset (0);
    weather_query (WEATHER_QUERY_ID_TEXT);
    ack_delay = 20;
    send ("N0j0001");
    snprintf (n, sizeof (n), "angenommen nach %u ms", (unsigned) dauer);
    CHECK (n, angenommen == 1 && dauer < 100 && var_send_timeout_cnt == 0);

    printf ("\n%d Faelle, %d fehlgeschlagen\n", cases, fails);
    return fails ? 1 : 0;
}
