/* Pruefstand A62 (Paket 2026-10-11, S1.1): Netzzeit mit Aera-Rechnung und Bestaetigungsregel.
 *
 * Geprueft wird der WOERTLICHE ESP8266_TIME-Zweig aus main.c (zweig.c) samt timeserver.c und
 * my_gmtime() aus base.c -- nicht eine Nachbildung. Massstab ist ref.h aus ref.py (Python
 * datetime), unabhaengig vom C-Code. Beobachtet wird, was auch am Geraet sichtbar ist:
 * RTC-Schreiben, gmain, net_time_countdown danach, die Logzeilen.
 *
 *   AKN.1  Parser und Gueltigkeit: jede Referenzzahl n als Folge (n-15, n) im Abstand 15 s,
 *          Felder von n gegen Python; Formfehler zweimal, nie uebernommen
 *   AKN.2  Zeitzone beim Laden: tz[0] x tz[1]
 *   AKN.5  Bestaetigungsregel mit nachgebildeter uptime, Zustand nach Reset
 *
 * Der Stand vor A62 kennt timeserver_check_net_time() nicht; dann fehlt HAVE_A62, ein "Reset"
 * ist leer, und der Pruefstand laeuft trotzdem durch -- die Gegenprobe soll FEHL zeigen, nicht
 * "kaputt". */
#ifdef PRUEFSTAND_SCHNELLE_TYPEN
#include "pruefstand.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

/* ---- Umgebung fuer timeserver.c ---- */
#include "esp8266.h"
STUB_ESP8266        esp8266;
uint_fast8_t        eep_is_up = 1;
static uint8_t      eep_tz[2];
uint_fast8_t eep_read (uint_fast16_t off, uint8_t * buf, uint_fast16_t len)
{
    if (off == 116) memcpy (buf, eep_tz, len); else memset (buf, 'x', len);
    return 1;
}
uint_fast8_t eep_write (uint_fast16_t off, uint8_t * buf, uint_fast16_t len) { (void) off; (void) buf; (void) len; return 1; }
void esp8266_send_cmd (const char * a, const char * b, uint_fast8_t c) { (void) a; (void) b; (void) c; }

static char         logbuf[4096];
void stub_log (const char * s) { strncat (logbuf, s, sizeof (logbuf) - strlen (logbuf) - 2); strcat (logbuf, "\n"); }
void log_printf (const char * fmt, ...)
{
    char z[256]; va_list ap; va_start (ap, fmt); vsnprintf (z, sizeof (z), fmt, ap); va_end (ap);
    { char * e = strstr (z, "\r\n"); if (e) *e = '\0'; }
    stub_log (z);
}

#include "timeserver.c"

/* ---- Umgebung fuer den Zweig aus main.c ---- */
#define DISPLAY_CLOCK_FLAG_UPDATE_ALL   0x07
static uint_fast8_t                 display_clock_flag;
static volatile uint_fast16_t       net_time_countdown;
volatile uint32_t                   uptime;
static const char * const           wdays_en[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static struct { struct tm tm; uint_fast16_t year; uint_fast8_t month, mday, wday, hour, minute, second; } gmain;
static struct { uint_fast8_t rtc_is_up; } grtc = { 1 };
static int          rtc_schreib;
static struct tm    rtc_tm;
static void rtc_set_date_time (struct tm * t) { rtc_schreib++; rtc_tm = *t; }
static void var_send_tm (void) { }
/* A69 (S1.3, Ent-2 (2)): war die RTC beim Start nicht gueltig (night_wm leer), wertet der Zweig das
 * Nachtfenster beim ersten UEBERNOMMENEN TIME einmal aus. Attrappe zaehlt die Aufrufe und setzt
 * night_wm wie das Original. Ein Stand ohne diesen Aufruf faellt in "Ent-2 (2)" durch. */
static uint_fast16_t night_wm = 0xFFFF;
static int night_apply_n, night_apply_ok;
static void night_apply (uint_fast16_t skipped, uint_fast8_t at_start)
{
    night_apply_n++; night_apply_ok = (skipped == 0xFFFF && at_start == 1);
    night_wm = 1234;
}

static void zweig (uint_fast8_t msg_rtc)
{
    switch (msg_rtc)
    {
#include "zweig.c"
    }
}

#include "ref.h"

/* ---- Messung ---- */
typedef struct { int acc, ungueltig, sprung, settime, rtc; unsigned cd; int felder_ok, nacht; } ERG;
static unsigned long faelle, fehl;
static char kat_name[16][40]; static unsigned long kat_n[16], kat_f[16]; static int kat;

static void pruef (int ok, const char * was, const char * wert)
{
    faelle++; kat_n[kat]++;
    if (! ok) { fehl++; kat_f[kat]++; if (kat_f[kat] <= 4) printf ("  FEHL  [%s] %s bei \"%s\"\n", kat_name[kat], was, wert); }
}
static void neustart (void)            /* STM-Reset: .bss ist null, die RTC laeuft weiter */
{
#ifdef HAVE_A62
    memset (&net_time, 0, sizeof (net_time));
#endif
}
static ERG feed (const char * s, uint32_t up)
{
    ERG e;
    strncpy (esp8266.u.time, s, ESP8266_MAX_TIME_LEN); esp8266.u.time[ESP8266_MAX_TIME_LEN] = 0;   /* wie esp8266_copy() */
    uptime = up; net_time_countdown = 3799; rtc_schreib = 0; logbuf[0] = 0; night_apply_n = 0; night_apply_ok = 0;
    memset (&gmain, 0xA5, sizeof (gmain));
    timeserver.timezone = 0; timeserver.observe_summertime = 0;
    zweig (ESP8266_TIME);
    e.rtc = rtc_schreib; e.cd = (unsigned) net_time_countdown;
    e.acc = (rtc_schreib == 1);
    e.ungueltig = strstr (logbuf, "time: ungueltig, verworfen\n") != NULL && (logbuf == strstr (logbuf, "time: ungueltig, verworfen\n"));
    e.sprung = strstr (logbuf, "time: sprung, bestaetigung angefordert\n") != NULL;
    e.settime = strstr (logbuf, "cmd: set time to ") != NULL;
    e.felder_ok = 0;
    e.nacht = night_apply_n;
    return e;
}
static const REF * suche (uint32_t n) { int i; for (i = 0; i < REF_N; i++) if (ref[i].n == n) return &ref[i]; return NULL; }
static int felder (const REF * r)
{
    const struct tm * t = &rtc_tm;
    return t->tm_year + 1900 == r->f[0] && t->tm_mon + 1 == r->f[1] && t->tm_mday == r->f[2] && t->tm_hour == r->f[3]
        && t->tm_min == r->f[4] && t->tm_sec == r->f[5] && t->tm_wday == r->f[6] && t->tm_yday == r->f[7]
        && gmain.year == (unsigned) r->f[0] && gmain.month == (unsigned) r->f[1] && gmain.mday == (unsigned) r->f[2]
        && gmain.hour == (unsigned) r->f[3] && gmain.minute == (unsigned) r->f[4] && gmain.second == (unsigned) r->f[5];
}
static char * txt (uint32_t n) { static char b[4][16]; static int i; i = (i + 1) & 3; snprintf (b[i], 16, "%lu", (unsigned long) n); return b[i]; }

/* Erwartung: nicht uebernommen, Sprung gemeldet, Schnellabfrage ja/nein */
static void soll_sprung (ERG e, int schnell, const char * w)
{
    pruef (! e.acc && e.rtc == 0, "Sprung uebernommen", w);
    pruef (e.sprung && ! e.ungueltig, "Logzeile 'time: sprung, bestaetigung angefordert' fehlt", w);
    pruef (e.cd == (schnell ? 15u : 3799u), schnell ? "keine Schnellabfrage (Countdown != 15)" : "Countdown veraendert", w);
}
static void soll_acc (ERG e, uint32_t n, const char * w)
{
    const REF * r = suche (n);
    pruef (e.acc && ! e.sprung && ! e.ungueltig, "nicht sofort uebernommen", w);
    pruef (e.settime, "Zeile 'cmd: set time to' fehlt (normaler Bau)", w);
    pruef (e.acc && e.cd == 3600u + 180 + 40 - rtc_tm.tm_sec, "Countdown nach Uebernahme falsch", w);
    if (r) pruef (e.acc && felder (r), "Felder != Python datetime", w);
}
static void soll_ungueltig (ERG e, const char * w)
{
    pruef (! e.acc && e.rtc == 0, "ungueltiger Text uebernommen", w);
    pruef (e.ungueltig && ! e.sprung, "Logzeile 'time: ungueltig, verworfen' fehlt", w);
    pruef (e.cd == 3799u, "Countdown veraendert", w);
}
static void neue_kat (const char * n) { kat++; strncpy (kat_name[kat], n, 39); }

int main (void)
{
    const uint32_t N0 = 4000648905u;                 /* 2026-10-11, gueltig, Aera 0 */
    uint32_t t, i;
    int k;
    ERG e;

    /* ---- AKN.1: jede Referenzzahl als Folge (n-15, n) ---- */
    neue_kat ("AKN.1 Zahl");
    for (i = 0; i < REF_N; i++)
    {
        uint32_t n = ref[i].n; char w[40];
        snprintf (w, sizeof w, "%s nach %s", txt (n), txt (n - 15));
        neustart ();
        e = feed (txt (n - 15), 1000);
        if (ref[i].g_vor) soll_sprung (e, 1, w); else soll_ungueltig (e, w);
        e = feed (txt (n), 1015);
        if (! ref[i].gueltig) soll_ungueltig (e, w);
        else if (ref[i].g_vor) { soll_acc (e, n, w); }
        else soll_sprung (e, 1, w);
    }
    neue_kat ("AKN.1 Form");
    for (k = 0; k < FORM_N; k++)
    {
        neustart ();
        e = feed (form[k], 1000); soll_ungueltig (e, form[k]);
        e = feed (form[k], 1015); soll_ungueltig (e, form[k]);
        e = feed (txt (N0), 1030); soll_sprung (e, 1, form[k]);        /* Formfehler hinterlaesst keinen Kandidaten */
    }

    /* ---- AKN.5: Bestaetigungsregel ---- */
    neue_kat ("AKN.5 Start n, n+15");
    neustart (); soll_sprung (feed (txt (N0), 1000), 1, "start n");
    soll_acc (feed (txt (N0 + 15), 1015), N0 + 15, "start n+15");

    neue_kat ("AKN.5 doppelte 0");
    neustart ();
    for (k = 0; k < 4; k++) soll_sprung (feed ("0", 1000 + 15 * k), k < 3, "0 nach Start");
    soll_sprung (feed ("0", 1045 + 3800), 0, "0 regulaer");
    neustart (); soll_sprung (feed ("0", 1000), 1, "0"); soll_sprung (feed ("0", 1000), 1, "0 gleiche Sekunde");

    neue_kat ("AKN.5 stuendlich");
    neustart (); feed (txt (N0), 1000); soll_acc (feed (txt (N0 + 15), 1015), N0 + 15, "aufbau");
    t = 1015; uint32_t n = N0 + 15;
    t += 3800; soll_acc (feed (txt (n + 3800 + 30), t), n + 3800 + 30, "+30 s"); n += 3800 + 30;
    t += 3800; soll_acc (feed (txt (n + 3800 - 30), t), n + 3800 - 30, "-30 s"); n += 3800 - 30;
    t += 3800; soll_acc (feed (txt (n + 3800 + 600), t), n + 3800 + 600, "+600 s"); n += 3800 + 600;
    t += 3800; soll_acc (feed (txt (n + 3800 - 600), t), n + 3800 - 600, "-600 s"); n += 3800 - 600;
    t += 3800; soll_sprung (feed (txt (n + 3800 + 601), t), 1, "+601 s");
    soll_acc (feed (txt (n + 3815), t + 15), n + 3815, "danach wieder im Takt"); n += 3815; t += 15;
    t += 3800; soll_sprung (feed (txt (n + 3800 - 601), t), 1, "-601 s");

    neue_kat ("AKN.5 Einzelfehlwert");
    neustart (); feed (txt (N0), 1000); feed (txt (N0 + 15), 1015);
    soll_sprung (feed ("400064890", 4815), 1, "verstuemmelt (Ziffer fehlt)");
    soll_acc (feed (txt (N0 + 3830), 4830), N0 + 3830, "danach regulaer");
    soll_sprung (feed ("400064890", 8630), 1, "Schnellabfragen nach Uebernahme wieder frei");

    neue_kat ("AKN.5 echter Sprung");
    neustart (); feed (txt (N0), 1000); feed (txt (N0 + 15), 1015);
    soll_sprung (feed (txt (N0 + 15 + 3800 + 5000), 4815), 1, "Sprung +5000");
    soll_acc (feed (txt (N0 + 15 + 3815 + 5000), 4830), N0 + 15 + 3815 + 5000, "bestaetigt");

    neue_kat ("AKN.5 vier Fehlwerte");
    neustart (); feed (txt (N0), 1000); feed (txt (N0 + 15), 1015);
    for (k = 0; k < 4; k++) soll_sprung (feed (txt (100000000u * (k + 1)), 4815 + 15 * k), k < 3, "Fehlwert");
    soll_acc (feed (txt (N0 + 15 + 3800 + 60 + 3800), 4815 + 60 + 3800), N0 + 15 + 3800 + 60 + 3800, "regulaer danach");
    soll_sprung (feed ("1", 4815 + 60 + 3815), 1, "Zaehler zurueckgesetzt");

    neue_kat ("AKN.5 nach drei Fehlversuchen");
    neustart ();
    for (k = 0; k < 4; k++) soll_sprung (feed (txt (100000000u * (k + 1)), 1000 + 15 * k), k < 3, "Fehlwert nach Start");
    soll_sprung (feed (txt (N0), 1045 + 3800), 0, "regulaer, Kandidat");
    soll_acc (feed (txt (N0 + 3802), 1045 + 7602), N0 + 3802, "naechste regulaere bestaetigt (+2 s Gang)");

    neue_kat ("AKN.5 Aerengrenze");
    neustart (); feed ("4294966985", 1000); soll_acc (feed ("4294967000", 1015), 4294967000u, "aufbau Aera 0");
    soll_acc (feed ("3304", 4615), 3304, "Aera 1 sofort");
    neustart (); feed ("4294967290", 1000); soll_acc (feed ("9", 1015), 9, "Bestaetigung ueber die Grenze");

    neue_kat ("AKN.5 Reset");
    neustart (); feed (txt (N0), 1000); feed (txt (N0 + 15), 1015);
    neustart (); soll_sprung (feed (txt (N0 + 3815), 3), 1, "nach Reset Bestaetigung noetig");
    soll_acc (feed (txt (N0 + 3830), 18), N0 + 3830, "nach Reset bestaetigt");

    neue_kat ("AKN.5 Abstand und Toleranz");
    neustart (); feed (txt (N0), 1000); soll_sprung (feed (txt (N0 + 3), 1003), 1, "Bestaetigung nach 3 s");
    soll_acc (feed (txt (N0 + 18), 1018), N0 + 18, "nach 15 s");
    neustart (); feed (txt (N0), 1000); soll_acc (feed (txt (N0 + 20), 1015), N0 + 20, "+5 s");
    neustart (); feed (txt (N0), 1000); soll_acc (feed (txt (N0 + 10), 1015), N0 + 10, "-5 s");
    neustart (); feed (txt (N0), 1000); soll_sprung (feed (txt (N0 + 21), 1015), 1, "+6 s");
    neustart (); feed (txt (N0), 1000); soll_sprung (feed (txt (N0 + 9), 1015), 1, "-6 s");

    neue_kat ("AKN.5 ungueltig stoert nicht");
    neustart (); feed (txt (N0), 1000); feed (txt (N0 + 15), 1015);
    soll_ungueltig (feed ("", 4815), "leer");
    soll_ungueltig (feed ("3000000000", 4830), "Totbereich");
    soll_acc (feed (txt (N0 + 15 + 3830), 4845), N0 + 15 + 3830, "danach sofort");

    neue_kat ("AKN.5 uptime-Umlauf");
    neustart (); feed (txt (N0), 0xFFFFFFF0u); soll_acc (feed (txt (N0 + 15), 0xFFFFFFFFu), N0 + 15, "aufbau");
    soll_acc (feed (txt (N0 + 3815), 3799), N0 + 3815, "ueber den Umlauf");

    /* ---- Ent-2 (2): Nachtfenster beim ersten uebernommenen TIME (A69, S1.3) ---- */
    neue_kat ("Ent-2 (2) erstes TIME");
    neustart (); night_wm = 0xFFFF;                                         /* RTC beim Start nicht gueltig */
    e = feed ("", 1000);                pruef (e.nacht == 0, "ungueltiges TIME wertet das Nachtfenster aus", "Ent-2 (2)");
    e = feed (txt (N0), 1015);          pruef (e.nacht == 0, "unbestaetigter Sprung wertet das Nachtfenster aus", "Ent-2 (2)");
    e = feed (txt (N0 + 15), 1030);     pruef (e.acc && e.nacht == 1 && night_apply_ok, "erstes uebernommenes TIME: night_apply (0xFFFF, 1) nicht genau einmal", "Ent-2 (2)");
    pruef (night_wm != 0xFFFF, "night_wm danach nicht gesetzt", "Ent-2 (2)");
    e = feed (txt (N0 + 3815), 4830);   pruef (e.acc && e.nacht == 0, "zweites uebernommenes TIME wertet erneut aus", "Ent-2 (2)");
    neustart (); night_wm = 100;                                            /* RTC beim Start gueltig, schon ausgewertet */
    feed (txt (N0), 1000); e = feed (txt (N0 + 15), 1015);
    pruef (e.acc && e.nacht == 0, "RTC beim Start gueltig: erstes TIME wertet trotzdem aus", "Ent-2 (2)");

    /* ---- AKN.2: Zeitzone beim Laden ---- */
    neue_kat ("AKN.2 Zeitzone");
    {
        static const uint8_t z0[] = { 0x00, 0x01, 0x02, 0x03, '-', '+' };
        static const uint8_t z1[] = { 0, 1, 12, 13, 255 };
        unsigned a, b;
        for (a = 0; a < sizeof z0; a++) for (b = 0; b < sizeof z1; b++)
        {
            char w[32]; int neg = (z0[a] == '-') || (z0[a] != '+' && (z0[a] & 0x01));
            int soll = z1[b] > 12 ? 1 : (neg ? -(int) z1[b] : (int) z1[b]);
            snprintf (w, sizeof w, "tz={0x%02x,%u}", z0[a], z1[b]);
            eep_tz[0] = z0[a]; eep_tz[1] = z1[b]; logbuf[0] = 0; timeserver.timezone = 99;
            timeserver_read_data_from_eep ();
            pruef (timeserver.timezone == soll, "Zeitzone", w);
            pruef ((strstr (logbuf, "time: zeitzone ungueltig, vorgabe\n") != NULL) == (z1[b] > 12), "Logzeile Zeitzone", w);
        }
    }

    printf ("  Kategorien:\n");
    for (k = 1; k <= kat; k++) printf ("    %-34s %6lu Pruefungen, %lu FEHL\n", kat_name[k], kat_n[k], kat_f[k]);
    printf ("  Referenzzahlen %d (davon %d benannt), Formfehler %d\n", REF_N, REF_BENANNT, FORM_N);
    printf ("a62: %lu Pruefungen, %lu FEHL -- %s\n", faelle, fehl, fehl ? "FEHL" : "OK");
    return fehl ? 1 : 0;
}
