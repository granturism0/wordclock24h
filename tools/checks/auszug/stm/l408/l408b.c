/* Pruefstand L408 (b)+(c): var_send_buf() bei offline-ESP -- keine Sendung, keine Wartezeit; online
 * unveraendert. Echter Code: esp8266_get_message() aus esp8266.c und var_send_buf() samt Nachsendeliste
 * aus vars.c (Schnitt von tools/checks/auszug/stm/extract.py, derselbe wie AKS.5). Nachgebildet: UART,
 * Uhr (uptime = sim_ms / 1000, sim_ms laeuft NUR in schedule_esp8266_messages), der ESP.
 * Gemessene Wartezeit = Zuwachs von sim_ms: ohne Warteschleife exakt 0. */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pruefstand.h"
#include "esp8266.h"
#include "vars.h"

volatile uint32_t   uptime;
static uint32_t     sim_ms;
ESP8266_GLOBALS     esp8266;
uint_fast8_t        var_send_busy;
static unsigned     wd_reloads;
void watchdog_reload (void) { wd_reloads++; }
static void log_flush (void) {}
static void log_puts (const char * s) { (void) s; }
static char logbuf[8192];
static int  log_printf (const char * fmt, ...)
{
    va_list ap; size_t n = strlen (logbuf);
    va_start (ap, fmt); vsnprintf (logbuf + n, sizeof (logbuf) - n, fmt, ap); va_end (ap);
    return 0;
}
#define debug_log_printf(...)
#define debug_log_flush()
#define debug_log_message(s)

/* ---------- ESP ---------- */
#define MAXQ 1024
typedef struct { uint32_t due; char text[96]; } RX;
static RX       rxq[MAXQ];
static int      rx_tail, rx_cur = -1, rx_pos;
enum { ESP_LEBT, ESP_TOT, ESP_NAK };
static int      esp_zustand;
static int      cap_bei_n15;          /* L408-Bild: CAP kommt waehrend des Wartens auf N15, danach bootet der ESP */
static int      tx_zeilen;            /* var-Zeilen auf der UART */
static char     tx_letzte[128];
static void rx_push (uint32_t due, const char * t)
{
    if (rx_tail >= MAXQ) { fprintf (stderr, "rxq voll\n"); exit (2); }
    rxq[rx_tail].due = due; snprintf (rxq[rx_tail].text, sizeof rxq[0].text, "%s", t); rx_tail++;
}
static char txline[256]; static int txlen;
static void esp_zeile (const char * z)
{
    const char * star; char ack[16];
    if (strncmp (z, "var ", 4)) return;
    tx_zeilen++; snprintf (tx_letzte, sizeof tx_letzte, "%s", z);
    if (cap_bei_n15 && ! strncmp (z + 4, "N15", 3)) { rx_push (sim_ms + 5, "CAP var-crc\r\n"); esp_zustand = ESP_TOT; cap_bei_n15 = 0; return; }
    if (esp_zustand == ESP_TOT) return;
    star = strrchr (z, '*');
    if (star && strlen (star) == 5) snprintf (ack, sizeof ack, "ACK %02x\r\n", (unsigned) ((strtol (star + 1, 0, 16) >> 8) & 0xFF));
    else strcpy (ack, "ACK --\r\n");
    rx_push (sim_ms + 20, ack);
    rx_push (sim_ms + 20, esp_zustand == ESP_NAK ? "!v\r\n" : ".\r\n");
}
void esp8266_uart_puts (const char * s)
{
    for (; *s; s++) { txline[txlen++] = *s; if (*s == '\n') { txline[txlen - 2] = '\0'; esp_zeile (txline); txlen = 0; } }
}
void esp8266_uart_putc (uint_fast8_t c) { char b[2] = { (char) c, 0 }; esp8266_uart_puts (b); }
void esp8266_uart_flush (void) {}
static int rx_next (void)
{
    int i, best = -1;
    for (i = 0; i < rx_tail; i++) if (rxq[i].text[0] && rxq[i].due <= sim_ms && (best < 0 || rxq[i].due < rxq[best].due)) best = i;
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

uint_fast8_t schedule_esp8266_messages (void);
#include SNIPPET

static const char * nested_inject;    /* einmal verschachtelt senden, waehrend gewartet wird */
static int          nested_rc = -1;
uint_fast8_t schedule_esp8266_messages (void)
{
    uint_fast8_t r = esp8266_get_message ();
    if (nested_inject && var_send_cur_buf && ! esp8266.is_online)
    {
        char nb[64]; strcpy (nb, nested_inject); nested_inject = 0;
        nested_rc = var_send_buf (nb, 3);
    }
    if (r == ESP8266_OK || r == ESP8266_NAK) var_send_busy = 0;
    sim_ms++; uptime = sim_ms / 1000;
    return r;
}

/* ---------- Faelle ---------- */
static int faelle, fehl;
static void pruef (int ok, const char * w) { faelle++; if (! ok) { fehl++; printf ("  FEHL  %s\n", w); } }
static int slots_belegt (void) { int i, n = 0; for (i = 0; i < VAR_RETRY_SLOTS; i++) if (var_retry_slots[i].cmd[0]) n++; return n; }
static int log_zeilen (const char * was) { int n = 0; const char * p = logbuf; while ((p = strstr (p, was))) { n++; p++; } return n; }

static void frisch (int online, int zustand, int crc)
{
    memset (rxq, 0, sizeof rxq); rx_tail = 0; rx_cur = -1; txlen = 0;
    memset (var_retry_slots, 0, sizeof var_retry_slots);
    var_send_timeout_cnt = var_send_nested_cnt = 0;
    logbuf[0] = 0; tx_zeilen = 0; tx_letzte[0] = 0; wd_reloads = 0; cap_bei_n15 = 0; nested_inject = 0; nested_rc = -1;
    sim_ms = ((sim_ms / 1000) + 60) * 1000; uptime = sim_ms / 1000;
    esp8266.is_online = (uint_fast8_t) online; esp8266.cap_var_crc = (uint_fast8_t) crc; esp_zustand = zustand;
    var_send_reload_budget_reset ();
}
typedef struct { int rc; uint32_t ms; } ERG;
static ERG senden (const char * p, uint_fast8_t idlen)
{
    char b[96]; ERG e; uint32_t t0 = sim_ms;
    strcpy (b, p); e.rc = var_send_buf (b, idlen); e.ms = sim_ms - t0; return e;
}
#define W(...) (snprintf (was, sizeof was, __VA_ARGS__), was)

int main (void)
{
    char was[256]; ERG e, e2; int crc, z;
    static const char * zn[] = { "lebt", "tot", "nak" };

    /* (b) offline: nichts senden, nicht warten, nichts zaehlen, nichts vormerken -- egal, ob der ESP lebt */
    for (crc = 0; crc < 2; crc++) for (z = 0; z < 3; z++)
    {
        frisch (0, z, crc);
        e = senden ("N150020", 3); e2 = senden ("S0ameinhost", 3);
        pruef (tx_zeilen == 0, W ("(b) offline, ESP %s, crc %d: %d var-Zeilen gesendet, Soll 0", zn[z], crc, tx_zeilen));
        pruef (e.ms == 0 && e2.ms == 0, W ("(b) offline, ESP %s, crc %d: gewartet %u + %u ms, Soll 0", zn[z], crc, (unsigned) e.ms, (unsigned) e2.ms));
        pruef (e.rc == 0 && e2.rc == 0, W ("(b) offline: Rueckgabe %d/%d, Soll 0/0", e.rc, e2.rc));
        pruef (var_send_timeout_cnt == 0 && var_send_nested_cnt == 0, W ("(b) offline: v=%u/%u, Soll 0/0", (unsigned) var_send_timeout_cnt, (unsigned) var_send_nested_cnt));
        pruef (slots_belegt () == 0, W ("(b) offline: %d Eintraege vorgemerkt, Soll 0", slots_belegt ()));
        pruef (log_zeilen ("keine Quittung") == 0 && logbuf[0] == 0, W ("(b) offline: Logzeile '%.60s'", logbuf));
        pruef (wd_reloads == 0, W ("(b) offline: %u Watchdog-Reloads, Soll 0", wd_reloads));
    }

    /* (b) offline: ein vorgemerkter aelterer Wert wird verdraengt (Regel 1), der neue nicht vorgemerkt */
    frisch (1, ESP_TOT, 1);
    e = senden ("S0aalt", 3);
    pruef (slots_belegt () == 1, W ("(b) Vorbereitung: online, ESP tot -- %d vorgemerkt, Soll 1", slots_belegt ()));
    esp8266.is_online = 0; tx_zeilen = 0;
    e = senden ("S0aneu", 3);
    pruef (tx_zeilen == 0 && e.ms == 0 && slots_belegt () == 0,
           W ("(b) offline, gleiche Kennung: gesendet %d, gewartet %u ms, vorgemerkt %d -- Soll 0/0/0", tx_zeilen, (unsigned) e.ms, slots_belegt ()));

    /* (b) das L408-Bild: N15 wartet, CAP kommt, ESP bootet; N31 im selben Aufruf (var_send_rtc_temp_index) */
    for (crc = 0; crc < 2; crc++)
    {
        frisch (1, ESP_LEBT, crc); cap_bei_n15 = 1;
        e = senden ("N150020", 3);
        pruef (esp8266.is_online == 0, "(b) CAP waehrend N15 setzt nicht offline -- Pruefstand kaputt");
        pruef (var_send_timeout_cnt == 1 && e.ms >= 3000, W ("(b) N15 nach CAP: v=%u, %u ms (unveraendert: 1 Zeitueberschreitung, >= 3000 ms)", (unsigned) var_send_timeout_cnt, (unsigned) e.ms));
        z = tx_zeilen;
        e2 = senden ("N310001", 3);
        pruef (tx_zeilen == z && e2.ms == 0 && var_send_timeout_cnt == 1 && log_zeilen ("keine Quittung") == 1,
               W ("(b) N31 nach CAP, crc %d: gesendet %d, gewartet %u ms, v=%u, Timeout-Zeilen %d -- Soll 0, 0, 1, 1",
                  crc, tx_zeilen - z, (unsigned) e2.ms, (unsigned) var_send_timeout_cnt, log_zeilen ("keine Quittung")));
    }

    /* (b) verschachtelt: waehrend des Wartens geht der ESP offline, ein eingeschobenes Kommando sendet nichts */
    frisch (1, ESP_TOT, 1); cap_bei_n15 = 1; nested_inject = "S0bverschachtelt";
    e = senden ("N150020", 3);
    pruef (nested_inject == 0 && tx_zeilen == 1 && var_send_nested_cnt == 0 && nested_rc == 0,
           W ("(b) verschachtelt offline: Zeilen %d (Soll 1, nur N15), nested=%u (Soll 0), rc=%d", tx_zeilen, (unsigned) var_send_nested_cnt, nested_rc));

    /* (c) online: unveraendert */
    for (crc = 0; crc < 2; crc++)
    {
        frisch (1, ESP_LEBT, crc);
        e = senden ("N150020", 3);
        pruef (e.rc == 1 && tx_zeilen == 1 && e.ms > 0 && e.ms < 100 && wd_reloads == 1 && var_send_timeout_cnt == 0 && slots_belegt () == 0,
               W ("(c) online, ESP lebt, crc %d: rc=%d Zeilen=%d %u ms Reloads=%u v=%u vorgemerkt=%d", crc, e.rc, tx_zeilen, (unsigned) e.ms, wd_reloads, (unsigned) var_send_timeout_cnt, slots_belegt ()));
        pruef (! strncmp (tx_letzte, "var N150020", 11) && (crc ? (strlen (tx_letzte) == 16 && tx_letzte[11] == '*') : strlen (tx_letzte) == 11),
               W ("(c) online, crc %d: Zeile '%s'", crc, tx_letzte));
        e = senden ("N310001", 3);
        pruef (e.rc == 1 && tx_zeilen == 2, W ("(c) online: Folgesendung N31 rc=%d Zeilen=%d, Soll 1/2", e.rc, tx_zeilen));

        frisch (1, ESP_TOT, crc);
        e = senden ("S0ameinhost", 3);
        pruef (e.rc == 0 && tx_zeilen == 1 && e.ms >= 3000 && e.ms < 4100 && var_send_timeout_cnt == 1 && slots_belegt () == 1 &&
               log_zeilen ("keine Quittung") == 1 && strstr (logbuf, "vorgemerkt") && wd_reloads == 0,
               W ("(c) online, ESP tot, crc %d: rc=%d Zeilen=%d %u ms v=%u vorgemerkt=%d Reloads=%u Log '%.70s'", crc, e.rc, tx_zeilen, (unsigned) e.ms,
                  (unsigned) var_send_timeout_cnt, slots_belegt (), wd_reloads, logbuf));

        frisch (1, ESP_NAK, crc);
        e = senden ("S0ameinhost", 3);
        pruef (e.rc == 0 && tx_zeilen == 1 && e.ms < 100 && var_send_timeout_cnt == 0 && slots_belegt () == 1 && wd_reloads == 1,
               W ("(c) online, Ablehnung, crc %d: rc=%d Zeilen=%d %u ms v=%u vorgemerkt=%d Reloads=%u", crc, e.rc, tx_zeilen, (unsigned) e.ms,
                  (unsigned) var_send_timeout_cnt, slots_belegt (), wd_reloads));
    }

    printf ("l408b: %d Pruefungen, %d FEHL -- %s\n", faelle, fehl, fehl ? "FEHL" : "OK");
    return fehl ? 1 : 0;
}
