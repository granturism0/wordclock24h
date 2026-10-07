/* Pruefstand AKS.5 (S.16, A35 Teil 1): Zuordnung der Quittung ueber "ACK xy".
 *
 * Laesst den ECHTEN Firmware-Code laufen: esp8266_get_message() samt Praefixkette aus
 * src/esp8266/esp8266.c und var_send_buf()/var_retry_drain() aus src/vars/vars.c, ausgeschnitten
 * von extract.py. Nachgebildet sind nur die UART, die Uhr (uptime) und der ESP -- dieser so, wie
 * ESP-uclock.ino 3.2.25 antwortet: "ACK xy" (xy = hoeheres Byte der EMPFANGENEN Marke, klein,
 * "--" ohne Marke), dann "." oder "!v".
 *
 * Zieltypen (L256): pruefstand.h prueft die Breiten; uptime ist uint32_t wie auf dem STM32.
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pruefstand.h"

/* pruefstand.h gleicht unsigned long, size_t und Zeiger an, die schnellen Typen aber NICHT: Auf dem
 * Ziel sind uint_fast8_t und uint_fast16_t 32 Bit (arm-none-eabi-gcc: unsigned int), auf dem Mac
 * 8 bzw. 16 Bit. answer_pos, var_retry_attempts_in, esp8266_ack_* haengen daran. -DZIELTYPEN gleicht
 * sie an; der Pruefstand laeuft in beiden Uebersetzungen. */
#ifdef ZIELTYPEN
#define uint_fast8_t    uint32_t
#define uint_fast16_t   uint32_t
#define int_fast8_t     int32_t
typedef char zp_fast_pruefung[(sizeof (uint_fast8_t) == 4 && sizeof (uint_fast16_t) == 4) ? 1 : -1];
#endif
#include "esp8266.h"
#include "vars.h"

/* ---------- Umgebung des Ziels ---------- */
volatile uint32_t   uptime;
static uint32_t     sim_ms;
ESP8266_GLOBALS     esp8266;
uint_fast8_t        var_send_busy;
static unsigned     wd_reloads;
void watchdog_reload (void) { wd_reloads++; }
static int          verbose;

static void log_flush (void) {}
static void log_puts (const char * s) { if (verbose) fputs (s, stdout); }
static char logbuf[8192];                                           /* Mitschnitt aller log_printf-Zeilen (A20) */
static int  log_printf (const char * fmt, ...)
{
    va_list ap; size_t n = strlen (logbuf);
    va_start (ap, fmt); vsnprintf (logbuf + n, sizeof (logbuf) - n, fmt, ap); va_end (ap);
    if (verbose) fputs (logbuf + n, stdout);
    return 0;
}
#define debug_log_printf(...)
#define debug_log_flush()
#define debug_log_message(s)

/* ---------- ESP-Nachbildung ---------- */
#define MAXQ 4096
typedef struct { uint32_t due; char text[320]; int owner; } RX;     // owner: Nummer der Zeile, die der ESP quittiert
static RX       rxq[MAXQ];
static int      rx_head, rx_tail, rx_pos;
static int      last_owner = -1;                                    // Besitzer der zuletzt VOLLSTAENDIG gelesenen Zeile
static int      cur_owner  = -1;

typedef struct { char payload[96]; int sends; } SENT;
static SENT     sent[64];
static int      n_sent;

/* Verhalten je Nutzlast und Sendung: Verzoegerung in ms und Antwort. Vom Szenario gesetzt. */
typedef void (*PLAN) (const char * payload, int nth, uint32_t * delay, int * nak, const char ** raw);
static PLAN     plan;

static int
sent_index (const char * p)
{
    int i;
    for (i = 0; i < n_sent; i++) if (! strcmp (sent[i].payload, p)) return i;
    strcpy (sent[n_sent].payload, p); sent[n_sent].sends = 0; return n_sent++;
}

static void
rx_push (uint32_t due, const char * text, int owner)
{
    int i = rx_tail++;
    rxq[i].due = due; rxq[i].owner = owner; snprintf (rxq[i].text, sizeof (rxq[i].text), "%s", text);
}

static char     txline[256];
static int      txlen;

static void
esp_receive_line (const char * line)
{
    char        payload[96];
    const char *star;
    int         mark = -1, idx, nak = 0;
    uint32_t    delay = 20;
    const char *raw = 0;
    char        ack[16];

    if (strncmp (line, "var ", 4)) return;
    snprintf (payload, sizeof (payload), "%s", line + 4);
    star = strrchr (payload, '*');
    if (star && strlen (star) == 5) { mark = (int) strtol (star + 1, 0, 16); payload[star - payload] = '\0'; }
    idx = sent_index (payload);
    sent[idx].sends++;
    plan (payload, sent[idx].sends, &delay, &nak, &raw);
    if (raw) { rx_push (sim_ms + delay, raw, idx); return; }                     // Szenario liefert die Zeilen selbst
    if (mark >= 0) snprintf (ack, sizeof (ack), "ACK %02x\r\n", (mark >> 8) & 0xFF); else strcpy (ack, "ACK --\r\n");
    rx_push (sim_ms + delay, ack, idx);
    rx_push (sim_ms + delay, nak ? "!v\r\n" : ".\r\n", idx);
}

void esp8266_uart_puts (const char * s)
{
    for (; *s; s++) { txline[txlen++] = *s; if (*s == '\n') { txline[txlen - 2] = '\0'; esp_receive_line (txline); txlen = 0; } }
}
void esp8266_uart_putc (uint_fast8_t c) { char b[2] = { (char) c, 0 }; esp8266_uart_puts (b); }
void esp8266_uart_flush (void) {}

/* Die RX-Warteschlange wird nach Faelligkeit gelesen, nicht nach Einstellreihenfolge. */
static int
rx_next (void)
{
    int i, best = -1;
    for (i = rx_head; i < rx_tail; i++)
        if (rxq[i].text[0] && rxq[i].due <= sim_ms && (best < 0 || rxq[i].due < rxq[best].due)) best = i;
    return best;
}
static int  rx_cur = -1;
uint_fast8_t esp8266_uart_char_available (void) { return (rx_cur >= 0 || rx_next () >= 0) ? 1 : 0; }
uint_fast8_t esp8266_uart_poll (uint_fast8_t * ch)
{
    if (rx_cur < 0) { rx_cur = rx_next (); rx_pos = 0; if (rx_cur < 0) return 0; }
    *ch = (uint_fast8_t) rxq[rx_cur].text[rx_pos++];
    cur_owner = rxq[rx_cur].owner;
    if (*ch == '\n') last_owner = cur_owner;
    if (! rxq[rx_cur].text[rx_pos]) { rxq[rx_cur].text[0] = '\0'; rx_cur = -1; }
    return 1;
}

/* ---------- der echte Code ---------- */
uint_fast8_t schedule_esp8266_messages (void);
#include SNIPPET

/* ---------- Hauptloop-Nachbildung (main.c, schedule_esp8266_messages) ---------- */
static int      misattributed;      // Quittung beendete das Warten auf ein FREMDES Kommando
static int      quits_ok;

static const char * nested_inject;                                  /* A20: einmal verschachtelt dieselbe Kennung senden */

uint_fast8_t
schedule_esp8266_messages (void)
{
    uint_fast8_t r = esp8266_get_message ();

    if (nested_inject && var_send_cur_buf)
    {
        char nb[64];
        strcpy (nb, nested_inject); nested_inject = 0;
        var_send_buf (nb, 3);
    }

    if ((r == ESP8266_OK || r == ESP8266_NAK) && var_send_cur_buf)
    {
        quits_ok++;
        if (last_owner < 0 || strncmp (sent[last_owner].payload, var_send_cur_buf, strlen (sent[last_owner].payload)))
        {
            misattributed++;
            if (verbose) printf ("  [%u ms] Quittung von '%s' beendet Warten auf '%s'\n", sim_ms, last_owner >= 0 ? sent[last_owner].payload : "?", var_send_cur_buf);
        }
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
    while (sim_ms < end) { var_send_reload_budget_reset (); schedule_esp8266_messages (); var_retry_drain (); }
}

static void
send (const char * p)
{
    char b[96]; strcpy (b, p);
    var_send_buf (b, 3);
}

/* ---------- Szenarien ---------- */
static void
reset (PLAN p, int cap)
{
    memset (rxq, 0, sizeof (rxq)); rx_head = rx_tail = 0; rx_cur = -1; last_owner = cur_owner = -1;
    n_sent = 0; misattributed = 0; quits_ok = 0; plan = p; txlen = 0;
    memset (var_retry_slots, 0, sizeof (var_retry_slots));
    var_retry_ok_cnt = var_retry_gaveup_cnt = var_retry_dropped_cnt = var_retry_toolong_cnt = 0;
    var_send_timeout_cnt = var_send_nested_cnt = 0;
#ifdef HAVE_ACK
    esp8266_ack_drops = 0;
#endif
    esp8266.cap_var_crc = (uint_fast8_t) cap;
    sim_ms = ((sim_ms / 1000) + 10) * 1000;                     // Start auf Sekundengrenze: Timeout genau 3000 ms
    uptime = sim_ms / 1000;
}

static int fails, cases;
static unsigned drops (void)
{
#ifdef HAVE_ACK
    return esp8266_ack_drops;
#else
    return 0;
#endif
}
static int sends_of (const char * p) { int i; for (i = 0; i < n_sent; i++) if (! strcmp (sent[i].payload, p)) return sent[i].sends; return 0; }

#define CHECK(name, cond) do { cases++; if (cond) printf ("  OK    %s\n", name); else { fails++; printf ("  FEHL  %s\n", name); } } while (0)

/* 1: verspaeteter Punkt. A antwortet nach 3500 ms (Timeout bei 3000), B nach 1000 ms. A's Punkt trifft
 *    ein, waehrend B wartet -- er darf B NICHT quittieren. */
static void plan_late_dot (const char * p, int nth, uint32_t * d, int * nak, const char ** raw)
{ (void) raw; *nak = 0; *d = 20; if (! strcmp (p, "N0a0001") && nth == 1) *d = 3500; if (! strcmp (p, "N0b0002")) *d = 1000; }
/* 2: verspaetete Abweisung. Wie 1, aber A wird beim ersten Mal mit "!v" abgewiesen. */
static void plan_late_nak (const char * p, int nth, uint32_t * d, int * nak, const char ** raw)
{ (void) raw; *nak = 0; *d = 20; if (! strcmp (p, "N0a0001") && nth == 1) { *d = 3500; *nak = 1; } if (! strcmp (p, "N0b0002")) *d = 1000; }
/* 3: unpassende Zuordnung, direkt vor dem Punkt (beschaedigte bzw. fremde Zeile). */
static void plan_wrong_tag (const char * p, int nth, uint32_t * d, int * nak, const char ** raw)
{ (void) p; *nak = 0; *d = 20; if (nth == 1) *raw = "ACK 00\r\n.\r\n"; }
/* 4: Zuordnung ohne folgende Quittung -- eine andere Zeile kommt dazwischen. Sie ist damit verbraucht. */
static void plan_consumed (const char * p, int nth, uint32_t * d, int * nak, const char ** raw)
{ (void) p; (void) nth; *nak = 0; *d = 20; *raw = "ACK 00\r\nOK time\r\n.\r\n"; }
/* 5: "ACK --", ESP ohne Zuordnung, verstuemmelte ACK-Zeilen: alle wie heute. */
static void plan_dashes (const char * p, int nth, uint32_t * d, int * nak, const char ** raw)
{ (void) p; (void) nth; *nak = 0; *d = 20; *raw = "ACK --\r\n.\r\n"; }
static void plan_noack (const char * p, int nth, uint32_t * d, int * nak, const char ** raw)
{ (void) p; (void) nth; *nak = 0; *d = 20; *raw = ".\r\n"; }
static void plan_garbled (const char * p, int nth, uint32_t * d, int * nak, const char ** raw)
{ (void) p; *nak = 0; *d = 20; *raw = (nth & 1) ? "ACK 0\r\n.\r\n" : "ACK zz\r\n.\r\n"; }
/* 6: gesunder Stoss. */
static void plan_ok (const char * p, int nth, uint32_t * d, int * nak, const char ** raw)
{ (void) p; (void) nth; (void) raw; *nak = 0; *d = 20; }

#ifdef A13
/* A13 + A47 (S.19b): Jede Kopie in die Union esp8266.u ist danach abgeschlossen. Die Union wird
 * vorher mit 'X' gefuellt -- das stellt den Rest eines frueheren, laengeren Inhalts nach. Je Pfad
 * eine ueberlange Zeile (240 Zeichen Nutzlast); danach muss strlen(Feld) genau die Feldgrenze sein. */
static void plan_ok2 (const char * p, int nth, uint32_t * d, int * nak, const char ** raw)
{ (void) p; (void) nth; (void) raw; *nak = 0; *d = 20; }

static void
pfad (const char * name, const char * prefix, const char * feld, size_t grenze, int want_rtc)
{
    char    line[300];
    char    n[96];
    int     r;
    size_t  k;

    reset (plan_ok2, 0);
    memset (&esp8266.u, 'X', sizeof (esp8266.u));
    k = strlen (prefix);
    memcpy (line, prefix, k);
    memset (line + k, 'a', 240 - k);
    strcpy (line + 240, "\r\n");
    rx_push (sim_ms, line, -1);
    r = esp8266_get_message ();
    snprintf (n, sizeof (n), "%-10s Rueckgabe %d", name, want_rtc);  CHECK (n, r == want_rtc);
    snprintf (n, sizeof (n), "%-10s Feld abgeschlossen, strnlen = %u", name, (unsigned) grenze);
    CHECK (n, strnlen (feld, sizeof (esp8266.u)) == grenze);
}

int
main (void)
{
    printf ("[A13+A47] ueberlange Zeile je Pfad, Feld danach abgeschlossen\n");
    pfad ("FILE",       "FILE ",       esp8266.u.filedata, ESP8266_MAX_CMD_LEN,     ESP8266_FILEDATA);
    pfad ("ICON",       "ICON ",       esp8266.u.filedata, ESP8266_MAX_CMD_LEN,     ESP8266_ICONDATA);
    pfad ("DISP",       "DISP ",       esp8266.u.disp,     ESP8266_DISP_LEN,        ESP8266_DISP);
    pfad ("TIME",       "TIME ",       esp8266.u.time,     ESP8266_MAX_TIME_LEN,    ESP8266_TIME);
    pfad ("WEATHER",    "WEATHER ",    esp8266.u.weather,  ESP8266_MAX_WEATHER_LEN, ESP8266_WEATHER);
    pfad ("WEATHER_FC", "WEATHER_FC ", esp8266.u.weather,  ESP8266_MAX_WEATHER_LEN, ESP8266_WEATHER_FC);
    pfad ("OPEN",       "OPEN ",       esp8266.u.filedata, ESP8266_MAX_CMD_LEN,     ESP8266_FILEOPEN);
    printf ("  -- schon vorher abgeschlossen, Regression --\n");
    pfad ("CMD",        "CMD ",        esp8266.u.cmd,      ESP8266_MAX_CMD_LEN,     ESP8266_CMD);
    pfad ("TABINFO",    "TABINFO ",    esp8266.u.tabinfo,  ESP8266_TABINFO_LEN,     ESP8266_TABINFO);
    pfad ("TABILLU",    "TABILLU ",    esp8266.u.tabillu,  ESP8266_TABILLU_LEN,     ESP8266_TABILLU);
    pfad ("TABH",       "TABH ",       esp8266.u.tabh,     ESP8266_TABH_LEN,        ESP8266_TABH);
    pfad ("TABM",       "TABM ",       esp8266.u.tabm,     ESP8266_TABM_LEN,        ESP8266_TABM);
    pfad ("WICON",      "WICON ",      esp8266.u.weather,  2,                       ESP8266_WEATHER_ICON);
    pfad ("WICON_FC",   "WICON_FC ",   esp8266.u.weather,  2,                       ESP8266_WEATHER_FC_ICON);
    printf ("\n%d Faelle, %d fehlgeschlagen\n", cases, fails);
    return fails ? 1 : 0;
}
#elif defined (A20)
/* A20 (S.19a): Die Timeout-Meldung nennt Kennung (mit Index) und Laenge, nie den Wert. Beide
 * Timeout-Zweige: (1) vorgemerkt, (2) "inzwischen neuer Wert" -- waehrend des Wartens trifft ein
 * Kommando ein, das dieselbe Variable neu setzt (verschachtelter var_send_buf(), superseded). */
static void plan_silent (const char * p, int nth, uint32_t * d, int * nak, const char ** raw)
{ (void) p; (void) nth; *nak = 0; *d = 20; *raw = "OK time\r\n"; }          /* nie eine Quittung */

static void
pruefe (const char * zweig, const char * geheim, const char * len)
{
    char n[96];
    printf ("  Log: %s", logbuf);
    snprintf (n, sizeof (n), "%s: Timeout gemeldet", zweig);           CHECK (n, strstr (logbuf, "keine Quittung") != 0);
    snprintf (n, sizeof (n), "%s: Wert NICHT im Log", zweig);          CHECK (n, strstr (logbuf, geheim) == 0);
    snprintf (n, sizeof (n), "%s: Kennung mit Index im Log", zweig);   CHECK (n, strstr (logbuf, "S1c") != 0);
    snprintf (n, sizeof (n), "%s: Laenge im Log (%s)", zweig, len);    CHECK (n, strstr (logbuf, len) != 0);
}

int
main (void)
{
    char    b[64];

    printf ("[A20] Timeout-Meldung ohne Wert\n");
    reset (plan_silent, 1); logbuf[0] = 0;
    strcpy (b, "S1cGeHeIm-Schluessel-123"); var_send_buf (b, 3);
    pruefe ("vorgemerkt", "GeHeIm", "len=24");

    reset (plan_silent, 1); logbuf[0] = 0;
    nested_inject = "S1cNeuerWert";
    strcpy (b, "S1cAlterGeheimwert"); var_send_buf (b, 3);
    CHECK ("Zweig neuer Wert wurde tatsaechlich durchlaufen", var_send_superseded == 1);
    pruefe ("neuer Wert", "AlterGeheimwert", "len=18");

    printf ("\n%d Faelle, %d fehlgeschlagen\n", cases, fails);
    return fails ? 1 : 0;
}
#else
int
main (int argc, char ** argv)
{
    int i;
    char b[16];
    verbose = (argc > 1);

    printf ("[1] verspaeteter Punkt (AKS.5)\n");
    reset (plan_late_dot, 1);
    send ("N0a0001"); send ("N0b0002"); main_loop (10000);
    CHECK ("keine Quittung beendet das Warten auf ein fremdes Kommando", misattributed == 0);
    CHECK ("A genau einmal nachgesendet (2 Sendungen)", sends_of ("N0a0001") == 2);
    CHECK ("B nicht nachgesendet (1 Sendung)", sends_of ("N0b0002") == 1);
    CHECK ("var_retry_ok_cnt steigt auf 1", var_retry_ok_cnt == 1);
    CHECK ("unpassende Quittung gezaehlt (1)", drops () == 1);

    /* Seit Review S.21 H1 (08.10.2026) geht ein !v IMMER als NAK durch, auch mit fremder
     * Zuordnung: Der ESP sendet bei !v die EMPFANGENE Marke zurueck, und ist deren hohes
     * Byte verfaelscht, wurde aus dem NAK sonst ein DROP -- 3 s ohne Reload. Der Preis ist
     * eine zusaetzliche, gleichwertige Nachsendung von B. Deshalb hier bewusst: A's !v
     * beendet B's Warten (misattributed 1), A und B je einmal nachgesendet, beide
     * quittiert. "Genau eine Nachsendung" gilt weiter fuer den PUNKT, siehe [1]. */
    printf ("[2] verspaetete Abweisung !v (AKS.5, nach H1)\n");
    reset (plan_late_nak, 1);
    send ("N0a0001"); send ("N0b0002"); main_loop (10000);
    CHECK ("A's !v beendet B's Warten als NAK (gewollt, H1)", misattributed == 1);
    CHECK ("hoechstens eine zusaetzliche Nachsendung: 4 Sendungen", sends_of ("N0a0001") + sends_of ("N0b0002") == 4);
    CHECK ("A zweimal, B zweimal gesendet", sends_of ("N0a0001") == 2 && sends_of ("N0b0002") == 2);
    CHECK ("beide nachgesendet und quittiert (var_retry_ok_cnt 2)", var_retry_ok_cnt == 2);
    CHECK ("kein DROP bei !v", drops () == 0);

    printf ("[3] unpassende Zuordnung vor dem Punkt\n");
    reset (plan_wrong_tag, 1);
    send ("N0c0003"); main_loop (10000);
    CHECK ("Quittung verworfen und gezaehlt (1)", drops () == 1);
    CHECK ("Kommando lief in den Timeout (1)", var_send_timeout_cnt == 1);
    CHECK ("einmal nachgesendet, dann quittiert", sends_of ("N0c0003") == 2 && var_retry_ok_cnt == 1);

    printf ("[4] Zuordnung ohne folgende Quittung ist verbraucht\n");
    reset (plan_consumed, 1);
    send ("N0d0004"); main_loop (5000);
    CHECK ("Punkt nach Zwischenzeile gilt wie heute", sends_of ("N0d0004") == 1 && var_send_timeout_cnt == 0);
    CHECK ("nichts verworfen", drops () == 0);

    printf ("[5] Rueckfall: ACK --, kein ACK, verstuemmeltes ACK, ohne Marke\n");
    reset (plan_dashes, 1); send ("N0e0005"); main_loop (5000);
    CHECK ("ACK -- : quittiert wie heute", sends_of ("N0e0005") == 1 && var_send_timeout_cnt == 0 && drops () == 0);
    reset (plan_noack, 1); send ("N0e0005"); main_loop (5000);
    CHECK ("ohne ACK-Zeile (alter ESP): wie heute", sends_of ("N0e0005") == 1 && var_send_timeout_cnt == 0 && drops () == 0);
    reset (plan_garbled, 1); send ("N0e0005"); send ("N0f0006"); main_loop (5000);
    CHECK ("ACK 0 / ACK zz: keine Zuordnung, wie heute", n_sent == 2 && var_send_timeout_cnt == 0 && drops () == 0);
    reset (plan_wrong_tag, 0); send ("N0c0003"); main_loop (5000);
    CHECK ("STM ohne Marke (cap 0): jede Zuordnung ignoriert", sends_of ("N0c0003") == 1 && var_send_timeout_cnt == 0 && drops () == 0);

    printf ("[6] gesunder Stoss, 60 Kommandos\n");
    reset (plan_ok, 1);
    for (i = 0; i < 60; i++) { sprintf (b, "N%02x%04x", i, i * 7); send (b); }
    main_loop (5000);
    CHECK ("alle beim ersten Mal quittiert, kein Timeout, nichts verworfen",
           n_sent == 60 && var_send_timeout_cnt == 0 && drops () == 0 && misattributed == 0 && quits_ok == 60);

    printf ("\n%d Faelle, %d fehlgeschlagen\n", cases, fails);
    return fails ? 1 : 0;
}
#endif
