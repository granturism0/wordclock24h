/* ERZEUGT von stm-developer-gen.py aus a69.c -- nicht von Hand aendern */
/* Pruefstand A70 (Paket 2026-10-13, S2.11): Randfaelle beim Nachholen des Nachtfensters (AKN.1-AKN.5),
 * auf der Grundlage von a69 (Paket 2026-10-11, S1.3) -- dessen Faelle laufen hier unveraendert mit:
 *
 *   AKW.1  night_last_event() gegen eine Minuten-Referenz: night_check_night_times() aus
 *          release/3.2.28 (night-alt.c, eingefroren) liefert je Wochenminute, ob dort ein Aus-
 *          bzw. Ein-Timer greift; das juengste Ereignis ergibt sich daraus Minute fuer Minute
 *          rueckwaerts (zwei Umlaeufe ueber 10'080 Minuten). Alle 10'080 Startpunkte je Tabelle.
 *          Dazu: neue night_check_night_times() (mit herausgeloester Wochentagspruefung) gleich der
 *          alten, jede Minute, beide Zustaende.
 *   AKW.2  night_apply() beim Start (nur Aus, Ambilight mit, Logzeile)
 *   AKW.3  Minutenzweig aus main.c woertlich: Vorwaertssprung, rueckwaerts, ohne Ereignis,
 *          Sommerzeit, verpasste Minute, zwei Ereignisse im Intervall, Tageswechsel
 * Gleichstand (Aus und Ein in derselben Minute) wird gezaehlt und benannt, nicht bewertet.
 * Neu (A70), Zeit simuliert in Sekunden (uptime), Ticker-Attrappe mit der echten Laufzeit
 * (Laenge + 2) x (8 + 1) x ticker_deceleration / 64 s:
 *   AKN.1 (c)  Start im Aus-Fenster, IP-Lauftext, Restore kommt nie  => aus nach Lauftext + 10 s, nicht vorher
 *   AKN.2 (d)  RTC ungueltig, erstes TIME waehrend/vor/nach dem IP-Lauftext => Lauftext laeuft zu Ende, dann aus
 *   AKN.3 (e)  Wetterticker waehrend das Aus auf den IP-Lauftext wartet => aus VOR dem Ticker, Ticker nicht gezeigt
 *   AKN.4 L399 nur DCF77 => night_wm gesetzt, Sprungerkennung greift; nur ein Aus
 * Die Faelle aus a69 laufen mit IP "203.0.113.200" und ticker_deceleration 16 (A70 rechnet die Frist
 * aus beidem; mit leerer Adresse und 0 laege sie vor dem Ende der 40-s-Attrappe). */
#ifdef PRUEFSTAND_SCHNELLE_TYPEN
#include "pruefstand.h"
#endif
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

uint_fast8_t eep_is_up = 0;
uint_fast8_t eep_read (uint_fast16_t o, unsigned char * b, uint_fast16_t l) { (void) o; (void) b; (void) l; return 0; }
uint_fast8_t eep_write (uint_fast16_t o, unsigned char * b, uint_fast16_t l) { (void) o; (void) b; (void) l; return 0; }
static char logbuf[2048];
void log_printf (const char * fmt, ...)
{
    char z[256]; va_list ap; va_start (ap, fmt); vsnprintf (z, sizeof z, fmt, ap); va_end (ap);
    strncat (logbuf, z, sizeof (logbuf) - strlen (logbuf) - 1);
}
#include "night.c"
#define night_check_night_times ref_check
#include "night-alt.c"
#undef night_check_night_times
#ifndef HAVE_A69
#define NIGHT_EVENT_OFF 1
#define NIGHT_EVENT_ON  2
static uint_fast8_t night_last_event (uint_fast8_t w, uint_fast16_t m, uint_fast16_t * b) { (void) w; (void) m; *b = 0xFFFF; return 0; }
#endif

/* ---- Umgebung fuer die Teile aus main.c ---- */
#define TRUE  1
#define FALSE 0
#define DISPLAY_CLOCK_FLAG_UPDATE_ALL   0x01
#define DISPLAY_CLOCK_FLAG_POWER_ON     0x02
#define DISPLAY_CLOCK_FLAG_POWER_OFF    0x04
static struct { uint_fast8_t wday, hour, minute, month, mday, second; uint_fast16_t year; struct tm tm; } gmain;
static struct { uint_fast8_t display_power_is_on, ambilight_power_is_on, do_display_icon, ticker_deceleration; } display;
static struct { uint_fast8_t is_online; char ipaddress[16]; union { char weather[64]; } u; } esp8266;
volatile uint32_t   uptime;
#define MAX_OVERLAYS 8
static uint_fast8_t var_sync_pending, ip_ticker_pending, pending_ticker_restore, show_overlay_idx = MAX_OVERLAYS;
static uint32_t     show_icon_stop_time;
/* Ticker-Attrappe wie display_set_ticker(): bei ausgeschaltetem Display tut sie NICHTS (display.c) */
static int ticker_rest, ticker_gezeigt, ticker_laenge = 40;     /* > 0: feste Laufzeit (a69), sonst echte Laufzeit */
static int ticker_dauer (const unsigned char * s)
{ int dec = display.ticker_deceleration ? display.ticker_deceleration : 1; return (((int) strlen ((const char *) s) + 2) * 9 * dec + 63) / 64; }
static void display_set_ticker (const unsigned char * s, uint_fast8_t w) { (void) w; if (display.display_power_is_on) { ticker_rest = ticker_laenge > 0 ? ticker_laenge : ticker_dauer (s); ticker_gezeigt++; } }
static uint_fast8_t display_ticker_active (void) { return ticker_rest > 0; }
static uint_fast8_t display_clock_flag, dcf77_enabled;
/* Sendeprotokoll (A68): D = Display-Power, A = Ambilight-Power, in Reihenfolge */
static char proto[64];
static void var_send_display_power (void) { strcat (proto, "D"); }
static void var_send_display_ambilight_power (void) { strcat (proto, "A"); }
static void display_set_ambilight_power (uint_fast8_t on) { display.ambilight_power_is_on = on ? 1 : 0; }
static void display_icon_cancel (void) { }                                  /* A80: set_display_power() ruft es beim Aus */
static void dcf77_update (void) { dcf77_enabled = ! display.display_power_is_on && ! display.ambilight_power_is_on; }   /* A82: wie main.c */
static int spd_n, spd_sync;
static uint_fast8_t set_display_power_echt (uint_fast8_t, uint_fast8_t);
static uint_fast8_t set_display_power (uint_fast8_t on, uint_fast8_t sync) { spd_n++; spd_sync = sync; return set_display_power_echt (on, sync); }
/* DCF77-Block (L399) */
static const char * wdays_en[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static uint_fast8_t status_led_cnt;
static struct { uint_fast8_t rtc_is_up; } grtc;
static int tm_gesendet, dcf_neu;
static struct tm dcf_tm;
static void display_set_status_led (uint_fast8_t r, uint_fast8_t g, uint_fast8_t b) { (void) r; (void) g; (void) b; }
static void rtc_set_date_time (struct tm * t) { (void) t; }
static void var_send_tm (void) { tm_gesendet++; }
static uint_fast8_t dcf77_time (struct tm * t) { if (! dcf_neu) return 0; dcf_neu = 0; *t = dcf_tm; return 1; }
#include "main-a70.c"

static unsigned long faelle, fehl, kat_n[24], kat_f[24], gleichstand, mit_ereignis, ohne_ereignis, tabellen;
static const char * kat_name[24]; static int kat;
static void neue_kat (const char * n) { kat_name[++kat] = n; }
static void pruef (int ok, const char * was)
{
    faelle++; kat_n[kat]++;
    if (! ok) { fehl++; kat_f[kat]++; if (kat_f[kat] <= 4) printf ("  FEHL  [%s] %s\n", kat_name[kat], was); }
}
static uint32_t xs = 20261011u;
static uint32_t zufall (void) { xs ^= xs << 13; xs ^= xs >> 17; xs ^= xs << 5; return xs; }
#define F_AUS(fr, to)   (0x80 | ((fr) << 3) | (to))
#define F_EIN(fr, to)   (0xC0 | ((fr) << 3) | (to))
static void leer (void) { memset (night_time, 0, sizeof night_time); }
static void setz (int i, unsigned f, unsigned hh, unsigned mm) { night_time[i].flags = f; night_time[i].minutes = hh * 60 + mm; }

/* ---- AKW.1 je Tabelle ---- */
static unsigned char maske[7 * 1440];
static void tabelle_pruefen (const char * name)
{
    static int dist[7 * 1440];
    int w, b, any = 0, d;
    char was[160];
    tabellen++;
    for (w = 0; w < 7 * 1440; w++)
    {
        unsigned wd = w / 1440, m = w % 1440, p;
        maske[w] = (ref_check (0, 1, wd, m) ? 1 : 0) | (ref_check (0, 0, wd, m) ? 2 : 0);
        any |= maske[w];
        for (p = 0; p < 2; p++)                     /* Refaktor: neue Pruefung == alte, beide Zustaende */
        {
            kat = 1;                                 /* Kategorie AKW.1 Refaktor */
            if ((night_check_night_times (0, p, wd, m) != 0) != (ref_check (0, p, wd, m) != 0))
            { snprintf (was, sizeof was, "%s: night_check_night_times weicht ab, w=%d p=%u", name, w, p); pruef (0, was); }
            else pruef (1, "");
        }
    }
    /* Abstand zum juengsten Ereignis, zwei Umlaeufe */
    d = -1;
    for (b = 0; b < 2 * 7 * 1440; b++)
    {
        w = b % (7 * 1440);
        if (maske[w]) d = 0; else if (d >= 0) d++;
        if (b >= 7 * 1440) dist[w] = d;
    }
    kat = 2;
    for (w = 0; w < 7 * 1440; w++)
    {
        uint_fast16_t back = 0x1234;
        uint_fast8_t ev = night_last_event (w / 1440, w % 1440, &back);
        if (! any)
        {
            ohne_ereignis++;
            snprintf (was, sizeof was, "%s: w=%d Ereignis gemeldet, Referenz keins", name, w);
            pruef (ev == 0, was);
        }
        else
        {
            int q = (w - dist[w] + 7 * 1440) % (7 * 1440);
            int soll = maske[q];
            mit_ereignis++;
            if (soll == 3) gleichstand++;
            snprintf (was, sizeof was, "%s: w=%d (Tag %d %02d:%02d) ev=%u back=%lu, Referenz Maske %d back=%d",
                      name, w, w / 1440, (w % 1440) / 60, w % 60, (unsigned) ev, (unsigned long) back, soll, dist[w]);
            pruef (ev != 0 && back == (uint_fast16_t) dist[w] && (soll & (ev == NIGHT_EVENT_ON ? 2 : 1)), was);
        }
    }
}

/* ---- Szenarien ---- */
static void zeit (unsigned wd, unsigned hh, unsigned mm) { gmain.wday = wd; gmain.hour = hh; gmain.minute = mm; }
static void tick (unsigned wd, unsigned hh, unsigned mm) { zeit (wd, hh, mm); minutenzweig (); }
/* tick0: erster Tick nach einem Start mit gueltiger Zeit -- night_wm steht dann auf dieser Minute */
static void tick0 (unsigned wd, unsigned hh, unsigned mm) { night_wm = wd * 1440 + hh * 60 + mm; tick (wd, hh, mm); }
static int aus_bei;
static void frisch (int disp) { night_off_ip = 0; ip_ticker_end = 0; display.ticker_deceleration = 16; strcpy (esp8266.ipaddress, "203.0.113.200"); display.do_display_icon = 0; ticker_laenge = 40; var_sync_pending = 0; grtc.rtc_is_up = 0; dcf_neu = 0; tm_gesendet = 0; aus_bei = -1; night_off_due = 0; ticker_rest = 0; ticker_gezeigt = 0; ip_ticker_pending = 0; pending_ticker_restore = 0; display_clock_flag = 0; gmain.year = 2026; uptime = 5; night_wm = 0xFFFF; display.display_power_is_on = disp; display.ambilight_power_is_on = 1; spd_n = 0; spd_sync = -1; logbuf[0] = 0; proto[0] = 0; esp8266.is_online = 1; memset (ambilight_night_time, 0, sizeof ambilight_night_time); }
static void start (unsigned wd, unsigned hh, unsigned mm) { zeit (wd, hh, mm); night_apply (0xFFFF, 1); }
static void tab_a (void) { leer (); setz (0, F_AUS (0, 6), 23, 0); setz (1, F_EIN (0, 6), 7, 0); }

int main (void)
{
    int t, i;
    char n[64];
    neue_kat ("AKW.1 Refaktor"); neue_kat ("AKW.1 juengstes Ereignis");

    /* Sondertabellen */
    leer (); tabelle_pruefen ("leer");
    leer (); for (i = 0; i < 8; i++) setz (i, F_AUS (0, 6) & 0x7F, 1, 0); tabelle_pruefen ("alle inaktiv");
    leer (); setz (0, F_AUS (1, 5), 23, 0); tabelle_pruefen ("Mo-Fr aus 23:00");
    leer (); setz (0, F_AUS (6, 3), 22, 30); setz (1, F_EIN (6, 3), 6, 15); tabelle_pruefen ("Sa-Mi Umlauf");
    leer (); setz (0, F_AUS (0, 0), 0, 0); setz (1, F_EIN (6, 6), 23, 59); tabelle_pruefen ("So 00:00, Sa 23:59");
    leer (); setz (0, F_AUS (7, 7), 1, 0); setz (1, F_EIN (7, 2), 2, 0); setz (2, F_AUS (2, 7), 3, 0); tabelle_pruefen ("Tag 7");
    leer (); for (i = 0; i < 8; i++) { night_time[i].flags = (i & 1) ? F_EIN (0, 6) : F_AUS (0, 6); night_time[i].minutes = 1440; } tabelle_pruefen ("alle 1440");
    leer (); setz (0, F_AUS (0, 6), 23, 0); night_time[1].flags = F_EIN (0, 6); night_time[1].minutes = 1440; tabelle_pruefen ("1440 neben 23:00");
    leer (); for (i = 0; i < 8; i++) setz (i, (i & 1) ? F_EIN (0, 6) : F_AUS (0, 6), 12, 0); tabelle_pruefen ("Gleichstand 8x 12:00");
    leer (); setz (0, F_EIN (3, 3), 9, 0); setz (1, F_AUS (3, 3), 9, 0); tabelle_pruefen ("Gleichstand Mi 09:00 Ein vor Aus");
    /* Zufallstabellen */
    for (t = 0; t < 300; t++)
    {
        leer ();
        for (i = 0; i < 8; i++)
        {
            uint32_t r = zufall ();
            night_time[i].flags = (uint_fast8_t) (((r & 7) < 5 ? 0x80 : 0) | ((r >> 3) & 0x40) | ((r >> 8) & 0x3F));
            night_time[i].minutes = ((r >> 16) % 100 == 0) ? 1440 : (r >> 16) % 1440;
        }
        snprintf (n, sizeof n, "zufall %d", t);
        tabelle_pruefen (n);
    }

    /* ---- AKW.2 Start ---- */
    neue_kat ("AKW.2 Start");
    tab_a ();
    frisch (1); start (3, 1, 0);
    pruef (! display.display_power_is_on && ! display.ambilight_power_is_on && spd_n == 1 && spd_sync == 1, "Start Mi 01:00: Display und Ambilight nicht aus");
    pruef (strstr (logbuf, "night: start aus (23:00)\r\n") != NULL, "Logzeile 'night: start aus (23:00)' fehlt");
    pruef (night_wm == 3 * 1440 + 60, "night_wm nach dem Start nicht gesetzt");
    frisch (1); start (3, 12, 0);  pruef (display.display_power_is_on && spd_n == 0 && ! logbuf[0], "Start 12:00 (juengstes = Ein): veraendert");
    frisch (0); start (3, 8, 0);   pruef (! display.display_power_is_on && spd_n == 0, "Start/erstes TIME mit Ein und Display aus: eingeschaltet (nur Aus gilt)");
    leer (); frisch (1); start (3, 1, 0); pruef (display.display_power_is_on && spd_n == 0, "leere Tabelle: veraendert");
    leer (); setz (0, F_AUS (0, 0), 23, 0); frisch (1); start (1, 1, 0);
    pruef (! display.display_power_is_on && strstr (logbuf, "night: start aus (23:00)") != NULL, "Start Mo 01:00, Aus So 23:00: nicht aus");
    leer (); setz (0, F_AUS (1, 1), 23, 0); frisch (1); start (1, 1, 0);
    pruef (! display.display_power_is_on, "Start Mo 01:00, Aus nur Mo 23:00 (vor 6 Tagen + 2 h): nicht aus");

    /* ---- AKW.3 Spruenge ---- */
    neue_kat ("AKW.3 Spruenge");
    tab_a ();
    frisch (1); tick0 (3, 22, 58); tick (3, 22, 59); tick (3, 23, 5);
    pruef (! display.display_power_is_on && ! display.ambilight_power_is_on && spd_n == 1, "Sprung ueber Aus 23:00: nicht aus");
    pruef (strstr (logbuf, "night: sprung aus (23:00)\r\n") != NULL, "Logzeile 'night: sprung aus (23:00)' fehlt");
    frisch (1); tick0 (3, 22, 59); tick (3, 23, 1);  pruef (! display.display_power_is_on, "verpasste Minute 23:00 (Hauptloop blockiert): nicht aus");
    frisch (1); tick0 (3, 22, 58); tick (3, 23, 0);  pruef (! display.display_power_is_on && spd_n == 1, "Sprung genau auf 23:00: nicht genau einmal geschaltet");
    frisch (1); tick0 (3, 22, 59); tick (3, 23, 0); display.display_power_is_on = 1; spd_n = 0; tick (3, 23, 10);
    pruef (display.display_power_is_on && spd_n == 0, "Aus 23:00 schon ausgewertet, danach von Hand an, Sprung 23:00->23:10: erneut ausgeschaltet");
    frisch (0); tick0 (3, 12, 0); tick (3, 14, 0);    pruef (! display.display_power_is_on && spd_n == 0, "Sprung ohne Ereignis: manueller Zustand nicht erhalten");
    frisch (1); tick0 (3, 12, 0); tick (3, 14, 0);    pruef (display.display_power_is_on && spd_n == 0, "Sprung ohne Ereignis (an): veraendert");
    frisch (0); tick0 (3, 23, 5); tick (3, 22, 30);   pruef (! display.display_power_is_on && spd_n == 0, "rueckwaerts: veraendert");
    frisch (1); tick0 (4, 7, 30); tick (4, 6, 50);    pruef (display.display_power_is_on && spd_n == 0, "rueckwaerts ueber Ein: veraendert");
    frisch (0); tick0 (4, 6, 58); tick (4, 7, 10);
    pruef (display.display_power_is_on && spd_n == 1 && spd_sync == 0 && display.ambilight_power_is_on, "Sprung ueber Ein 07:00: nicht ein oder Ambilight angefasst");
    pruef (strstr (logbuf, "night: sprung ein (07:00)\r\n") != NULL, "Logzeile 'night: sprung ein (07:00)' fehlt");
    leer (); setz (0, F_AUS (0, 6), 23, 0); setz (1, F_EIN (0, 6), 23, 30);
    frisch (1); tick0 (3, 22, 59); tick (3, 23, 40);  pruef (display.display_power_is_on && spd_n == 0, "zwei Ereignisse, juengstes Ein: veraendert");
    frisch (0); tick0 (3, 22, 59); tick (3, 23, 40);  pruef (display.display_power_is_on && spd_n == 1, "zwei Ereignisse, nur das juengste (Ein) gilt");
    leer (); setz (0, F_AUS (0, 0), 2, 30);
    frisch (1); tick0 (0, 1, 59); tick (0, 3, 0);     pruef (! display.display_power_is_on, "Sommerzeit 02:00->03:00 mit Timer 02:30: nicht aus");
    leer (); setz (0, F_AUS (0, 0), 0, 1);
    frisch (1); tick0 (6, 23, 59); tick (0, 0, 2);    pruef (! display.display_power_is_on, "Tageswechsel Sa 23:59 -> So 00:02, Timer So 00:01: nicht aus");
    tab_a ();
    frisch (1); tick0 (1, 12, 0); tick (5, 12, 0);    pruef (display.display_power_is_on && spd_n == 0, "Sprung ueber 3,5 Tage gilt als rueckwaerts: veraendert");
    frisch (1); tick0 (2, 0, 0); for (i = 1; i < 1440; i++) tick (2, i / 60, i % 60);
    pruef (spd_n == 1 && display.display_power_is_on == 0, "normaler Tag Minute fuer Minute: nicht genau eine Schaltung (07:00 Ein bei Display an wirkungslos, 23:00 aus)");
    frisch (1); tick (3, 22, 59); tick (3, 23, 5);
    pruef (display.display_power_is_on && spd_n == 0, "ohne gueltige Zeit (night_wm leer): Sprung ausgewertet");

    /* ---- AKB.1 Sendeprotokoll (A68) ---- */
    neue_kat ("AKB.1 Sendeprotokoll");
    tab_a ();
    frisch (1); start (3, 1, 0);                    pruef (! strcmp (proto, "DA"), "Start aus (night_apply): nicht genau D, A");
    frisch (1); tick0 (3, 22, 59); tick (3, 23, 0);  pruef (! strcmp (proto, "DA"), "Display-Aus-Timer 23:00: nicht genau D, A");
    frisch (1); tick0 (3, 22, 59); tick (3, 23, 5);  pruef (! strcmp (proto, "DA"), "Sprung ueber Aus (night_apply): nicht genau D, A");
    frisch (0); tick0 (3, 6, 59); tick (3, 7, 0);    pruef (! strcmp (proto, "D"), "Display-Ein-Timer (Ambilight bleibt): nicht genau D");
    frisch (0); tick0 (3, 6, 58); tick (3, 7, 10);   pruef (! strcmp (proto, "D"), "Sprung ueber Ein (night_apply): nicht genau D");
    frisch (1); set_display_power (FALSE, TRUE);     pruef (! strcmp (proto, "DA"), "RPC/IR set_display_power (x, TRUE): nicht genau D, A");
    frisch (1); esp8266.is_online = 0; start (3, 1, 0);
    pruef (! proto[0] && ! display.ambilight_power_is_on, "offline: gesendet oder nicht geschaltet");
    leer (); frisch (1);
    ambilight_night_time[0].flags = F_AUS (0, 6); ambilight_night_time[0].minutes = 22 * 60;
    tick0 (3, 21, 59); tick (3, 22, 0);
    pruef (! strcmp (proto, "A") && ! display.ambilight_power_is_on && display.display_power_is_on, "Ambilight-Aus-Timer: nicht genau A");
    frisch (1); ambilight_night_time[0].flags = F_AUS (0, 6); ambilight_night_time[0].minutes = 22 * 60; esp8266.is_online = 0;
    tick0 (3, 21, 59); tick (3, 22, 0);                pruef (! proto[0] && ! display.ambilight_power_is_on, "Ambilight-Timer offline: gesendet");

    /* ---- M1/M2 (Review S1.16): Start im Aus-Fenster, IP erst zeigen, dann aus ---- */
    neue_kat ("M1/M2 Start");
    {
        int s, flacker;
        #define LAUF(bis, ip_bei, eingriff) \
            for (flacker = 0, s = (int) uptime; s <= (bis); s++) { \
                uptime = s; if (s == (ip_bei)) ip_ticker_pending = 1; eingriff; \
                { uint_fast8_t vor = display.display_power_is_on; hauptloop_teile (); display_clock_flag = 0; \
                  if (ticker_rest > 0) ticker_rest--; if (vor != display.display_power_is_on) flacker++; } \
                if (display.display_power_is_on == 0 && ticker_rest > 0) pruef (0, "Display aus, waehrend der IP-Lauftext laeuft"); }

        /* M1: RTC nach leerer Batterie */
        tab_a (); frisch (1); gmain.year = 2000; zeit (6, 0, 0); startblock ();
        pruef (display.display_power_is_on && spd_n == 0 && night_wm == 0xFFFF && night_off_due == 0, "M1: RTC 2000-01-01 wertet das Nachtfenster aus");
        LAUF (200, 9999, ;)
        pruef (display.display_power_is_on && spd_n == 0, "M1: ohne gueltige Zeit spaeter geschaltet");
        gmain.year = 2026; zeit (3, 1, 0); if (night_wm == 0xFFFF) night_apply (0xFFFF, 1);   /* wie der TIME-Zweig (a62 prueft ihn woertlich) */
        pruef (! display.display_power_is_on && spd_n == 1 && strstr (logbuf, "night: start aus (23:00)") != NULL, "M1: erstes TIME schaltet nicht aus");

        /* M2 mit IP: Lauftext ab 50 s, laeuft 40 s -- ueber die Frist (65 s) hinaus */
        tab_a (); frisch (1); zeit (3, 1, 0); startblock ();
        pruef (display.display_power_is_on && spd_n == 0 && night_off_due == 5 + 60 && ! logbuf[0], "M2 mit IP: beim Start sofort geschaltet statt vorgemerkt");
        LAUF (89, 50, ;)
        pruef (ticker_gezeigt == 1, "M2 mit IP: IP-Lauftext nicht gezeigt");
        pruef (display.display_power_is_on && spd_n == 0, "M2 mit IP: vor Ende des Lauftexts ausgeschaltet (Frist darf nicht greifen)");
        LAUF (200, 9999, ;)
        pruef (! display.display_power_is_on && ! display.ambilight_power_is_on && spd_n == 1 && flacker <= 1, "M2 mit IP: nach dem Lauftext nicht genau einmal aus");
        pruef (strstr (logbuf, "night: start aus (23:00)\r\n") != NULL, "M2 mit IP: Logzeile 'night: start aus (23:00)' fehlt");
        pruef (night_off_due == 0, "M2 mit IP: Vormerkung nicht aufgeloest");

        /* M2 ohne IP: Frist */
        tab_a (); frisch (1); zeit (3, 1, 0); startblock ();
        LAUF (64, 9999, ;)
        pruef (display.display_power_is_on && spd_n == 0, "M2 ohne IP: vor der Frist ausgeschaltet");
        LAUF (200, 9999, ;)
        pruef (! display.display_power_is_on && spd_n == 1 && night_off_due == 0, "M2 ohne IP: nach der Frist nicht genau einmal aus");

        /* M2: Nutzer schaltet vorher (IR aus, IR an) */
        tab_a (); frisch (1); zeit (3, 1, 0); startblock ();
        LAUF (200, 50, if (s == 20) set_display_power (FALSE, TRUE); if (s == 25) set_display_power (TRUE, TRUE))
        pruef (display.display_power_is_on && spd_n == 2 && night_off_due == 0, "M2: Nutzer schaltet vorher -- Vormerkung nicht verworfen");

        /* M2: Ein-Timer 07:00 liegt zwischen Start (06:59) und Frist */
        tab_a (); frisch (1); zeit (3, 6, 59); startblock ();
        pruef (night_off_due != 0, "M2: Start 06:59 merkt kein Aus vor");
        LAUF (30, 9999, ;) tick (3, 7, 0); LAUF (200, 9999, ;)
        pruef (display.display_power_is_on && spd_n == 0 && night_off_due == 0, "M2: Ein-Timer dazwischen -- trotzdem ausgeschaltet");

        /* M2: Start ausserhalb des Fensters merkt nichts vor */
        tab_a (); frisch (1); zeit (3, 12, 0); startblock ();
        pruef (night_off_due == 0 && spd_n == 0, "M2: Start 12:00 merkt vor");
        LAUF (200, 50, ;)
        pruef (display.display_power_is_on && spd_n == 0 && ticker_gezeigt == 1, "M2: Start 12:00 geschaltet");
    }

    /* ================= A70 ================= */
    {
        int s, flacker, dec, tz, dauer, ip0;
        char was[200];
        /* LAUF2: wie LAUF, merkt die Sekunde des Ausschaltens (aus_bei); die Pruefung "aus, waehrend der IP-Lauftext
         * laeuft" macht jeder Fall selbst -- bei (e) ist genau das gewollt. Ein Schalten im Eingriff (TIME, Wetter) zaehlt mit. */
        #define LAUF2(bis, ip_bei, eingriff) \
            for (flacker = 0, s = (int) uptime; s <= (bis); s++) { \
                uint_fast8_t vor = display.display_power_is_on; \
                uptime = s; if (s == (ip_bei)) ip_ticker_pending = 1; eingriff; \
                { hauptloop_teile (); display_clock_flag = 0; \
                  if (ticker_rest > 0) ticker_rest--; if (vor != display.display_power_is_on) { flacker++; if (! display.display_power_is_on && aus_bei < 0) aus_bei = s; } } }
        #define IPDAUER() ticker_dauer ((const unsigned char *) "IP 203.0.113.200")

        /* ---- AKN.1 (c): Restore kommt nie ---- */
        neue_kat ("AKN.1 (c) ohne Restore");
        for (dec = 0; dec <= 16; dec++)
            for (ip0 = 30; ip0 <= 64; ip0++)
            {
                tab_a (); frisch (1); ticker_laenge = 0; display.ticker_deceleration = dec; zeit (3, 1, 0); startblock ();
                dauer = IPDAUER ();
                LAUF2 (400, ip0, if (s == ip0 - 5) display.do_display_icon = 1)          /* Icon haengt: Restore-Bedingung bleibt falsch */
                snprintf (was, sizeof was, "dec=%d IP ab %d s (Laufzeit %d s): aus bei %d, erwartet in [%d, %d]", dec, ip0, dauer, aus_bei, ip0 + dauer, ip0 + dauer + 11);
                pruef (aus_bei >= ip0 + dauer && aus_bei <= ip0 + dauer + 11 && spd_n == 1 && flacker == 1, was);
                pruef (strstr (logbuf, "night: start aus (23:00)\r\n") != NULL && night_off_due == 0 && night_off_ip == 0, "ohne Restore: Logzeile fehlt oder Vormerkung nicht aufgeloest");
            }
        /* Restore kommt: aus genau mit dem Ende des Lauftexts (wie a69/M2, mit echter Laufzeit) */
        for (dec = 0; dec <= 16; dec++)
        {
            tab_a (); frisch (1); ticker_laenge = 0; display.ticker_deceleration = dec; zeit (3, 1, 0); startblock ();
            dauer = IPDAUER ();
            LAUF2 (400, 50, ;)
            snprintf (was, sizeof was, "mit Restore, dec=%d: aus bei %d, erwartet %d", dec, aus_bei, 50 + dauer);
            pruef (aus_bei == 50 + dauer && spd_n == 1, was);
        }

        /* ---- AKN.2 (d): erstes TIME bei ungueltiger RTC ---- */
        neue_kat ("AKN.2 (d) erstes TIME");
        for (dec = 3; dec <= 16; dec += 13)
        {
            dauer = 0;
            for (tz = 51; tz < 51 + 45; tz++)                                            /* TIME waehrend des Lauftexts */
            {
                tab_a (); frisch (1); ticker_laenge = 0; display.ticker_deceleration = dec; gmain.year = 2000; zeit (3, 1, 0); startblock ();
                dauer = IPDAUER (); if (tz >= 50 + dauer) break;
                LAUF2 (400, 50, if (s == tz) { gmain.year = 2026; time_erst (); })
                snprintf (was, sizeof was, "dec=%d TIME bei %d s, IP 50..%d s: aus bei %d (Lauftext abgeschnitten?)", dec, tz, 50 + dauer, aus_bei);
                pruef (aus_bei >= 50 + dauer && aus_bei <= 50 + dauer + 11 && spd_n == 1 && ticker_gezeigt == 1, was);
                pruef (strstr (logbuf, "night: start aus (23:00)\r\n") != NULL, "TIME waehrend IP: Logzeile fehlt");
            }
            /* TIME, waehrend der IP-Lauftext aussteht (IPADDRESS gesehen, Vollabgleich laeuft noch) */
            tab_a (); frisch (1); ticker_laenge = 0; display.ticker_deceleration = dec; gmain.year = 2000; zeit (3, 1, 0); startblock ();
            LAUF2 (400, 9999, if (s == 45) { ip_ticker_pending = 1; var_sync_pending = 1; } if (s == 47) { gmain.year = 2026; time_erst (); } if (s == 52) var_sync_pending = 0;)
            snprintf (was, sizeof was, "dec=%d TIME bei ausstehendem IP-Lauftext: gezeigt %d, aus bei %d, erwartet in [%d, %d]", dec, ticker_gezeigt, aus_bei, 52 + dauer, 52 + dauer + 11);
            pruef (ticker_gezeigt == 1 && aus_bei >= 52 + dauer && aus_bei <= 52 + dauer + 11 && spd_n == 1, was);
            /* TIME nach dem IP-Lauftext: spaetestens zur Frist, sonst sofort */
            for (tz = 50 + dauer; tz < 50 + dauer + 20; tz++)
            {
                int bis;
                tab_a (); frisch (1); ticker_laenge = 0; display.ticker_deceleration = dec; gmain.year = 2000; zeit (3, 1, 0); startblock ();
                LAUF2 (400, 50, if (s == tz) { gmain.year = 2026; time_erst (); })
                bis = tz > 50 + dauer + 10 ? tz : 50 + dauer + 10;
                snprintf (was, sizeof was, "dec=%d TIME bei %d s nach dem Lauftext (Ende %d): aus bei %d, erwartet %d..%d", dec, tz, 50 + dauer, aus_bei, tz, bis);
                pruef (aus_bei >= tz && aus_bei <= bis && spd_n == 1, was);
            }
        }
        /* TIME ganz ohne IP-Lauftext: sofort wie bisher */
        tab_a (); frisch (1); gmain.year = 2000; zeit (3, 1, 0); startblock ();
        LAUF2 (100, 9999, if (s == 30) { gmain.year = 2026; time_erst (); })
        pruef (aus_bei == 30 && spd_n == 1, "TIME ohne IP-Lauftext: nicht sofort aus");
        /* TIME lange nach dem IP-Lauftext: sofort */
        tab_a (); frisch (1); gmain.year = 2000; zeit (3, 1, 0); startblock ();
        LAUF2 (400, 50, if (s == 300) { gmain.year = 2026; time_erst (); })
        pruef (aus_bei == 300 && spd_n == 1, "TIME lange nach dem IP-Lauftext: nicht sofort aus");
        /* TIME waehrend IP ausserhalb des Aus-Fensters: nichts vorgemerkt, nichts geschaltet */
        tab_a (); frisch (1); gmain.year = 2000; zeit (3, 12, 0); startblock ();
        LAUF2 (400, 50, if (s == 60) { gmain.year = 2026; time_erst (); })
        pruef (spd_n == 0 && display.display_power_is_on && night_off_due == 0 && night_wm == 3 * 1440 + 720, "TIME 12:00 waehrend IP: geschaltet oder vorgemerkt");
        /* TIME waehrend IP, Nutzer schaltet danach selbst an: Vormerkung verworfen */
        tab_a (); frisch (1); gmain.year = 2000; zeit (3, 1, 0); startblock ();
        LAUF2 (400, 50, if (s == 60) { gmain.year = 2026; time_erst (); } if (s == 70) set_display_power (TRUE, TRUE);)
        pruef (spd_n == 1 && display.display_power_is_on && night_off_due == 0 && night_off_ip == 0, "TIME waehrend IP, dann von Hand an: spaeter doch aus");

        /* ---- AKN.3 (e): fremder Ticker ---- */
        neue_kat ("AKN.3 (e) fremder Ticker");
        for (tz = 51; tz < 51 + 45; tz++)
        {
            tab_a (); frisch (1); ticker_laenge = 0; zeit (3, 1, 0); startblock ();
            dauer = IPDAUER (); if (tz >= 50 + dauer) break;
            LAUF2 (400, 50, if (s == tz) { strcpy (esp8266.u.weather, "Wetter: sonnig, 21 Grad, Wind 5 km/h"); pf_weather (); })
            snprintf (was, sizeof was, "Wetter bei %d s, IP 50..%d s: aus bei %d (erwartet %d), Ticker gezeigt %d (erwartet 1)", tz, 50 + dauer, aus_bei, tz, ticker_gezeigt);
            pruef (aus_bei == tz && ticker_gezeigt == 1 && spd_n == 1 && ! display.ambilight_power_is_on, was);
            pruef (strstr (logbuf, "night: start aus (23:00)\r\n") != NULL, "Wetter waehrend IP: Logzeile fehlt");
        }
        /* ohne vorgemerktes Aus: Wetter erscheint, nichts geschaltet */
        tab_a (); frisch (1); ticker_laenge = 0; zeit (3, 12, 0); startblock ();
        LAUF2 (400, 50, if (s == 60) { strcpy (esp8266.u.weather, "sonnig"); pf_weather (); })
        pruef (ticker_gezeigt == 2 && spd_n == 0 && display.display_power_is_on, "Wetter ohne Vormerkung: nicht gezeigt oder geschaltet");
        /* Ein-Timer zwischen Start und Wetter: neu ausgewertet, Display bleibt an, Wetter erscheint */
        tab_a (); frisch (1); ticker_laenge = 0; zeit (3, 6, 59); startblock ();
        LAUF2 (400, 50, if (s == 55) tick (3, 7, 0); if (s == 60) { strcpy (esp8266.u.weather, "sonnig"); pf_weather (); })
        pruef (ticker_gezeigt == 2 && spd_n == 0 && display.display_power_is_on && night_off_due == 0 && night_off_ip == 0, "Ein-Timer vor dem Wetter: trotzdem aus");
        /* Aus vorgemerkt, IP-Lauftext noch nicht da: Wetter erscheint, die 60-s-Frist gilt unveraendert */
        tab_a (); frisch (1); ticker_laenge = 0; zeit (3, 1, 0); startblock ();
        LAUF2 (400, 9999, if (s == 30) { strcpy (esp8266.u.weather, "Wetter: sonnig, 21 Grad, Wind 5 km/h, Regen 0 mm"); pf_weather (); })
        snprintf (was, sizeof was, "Wetter vor dem IP-Lauftext: aus bei %d, erwartet 65 (Frist, nicht verzoegert)", aus_bei);
        pruef (aus_bei == 65 && ticker_gezeigt == 1, was);

        /* ---- AKN.4 L399: nur DCF77 ---- */
        neue_kat ("AKN.4 L399 DCF77");
        #define DCF(wd, hh, mm) do { memset (&dcf_tm, 0, sizeof dcf_tm); dcf_tm.tm_year = 126; dcf_tm.tm_mon = 9; dcf_tm.tm_mday = 14; \
                                     dcf_tm.tm_wday = (wd); dcf_tm.tm_hour = (hh); dcf_tm.tm_min = (mm); dcf_neu = 1; dcf77_block (); } while (0)
        /* RTC ungueltig, kein TIME, Display und Ambilight aus (sonst empfaengt DCF77 nicht) */
        tab_a (); frisch (0); display.ambilight_power_is_on = 0; gmain.year = 2000; zeit (0, 0, 0); startblock ();
        LAUF2 (100, 9999, ;)
        DCF (3, 6, 58);
        pruef (night_wm == 3 * 1440 + 6 * 60 + 58 && spd_n == 0 && ! display.display_power_is_on, "DCF77-Zeit: night_wm nicht gesetzt oder geschaltet");
        tick (3, 7, 10);
        pruef (display.display_power_is_on && spd_n == 1 && strstr (logbuf, "night: sprung ein (07:00)\r\n") != NULL, "DCF77, danach Sprung ueber den Ein-Timer 07:00: Sprungerkennung greift nicht");
        /* im Ein-Fenster bei absichtlich ausgeschaltetem Display: nur ein Aus, also nichts */
        tab_a (); frisch (0); display.ambilight_power_is_on = 0; gmain.year = 2000; zeit (0, 0, 0); startblock ();
        DCF (3, 12, 0);
        pruef (! display.display_power_is_on && spd_n == 0 && night_wm == 3 * 1440 + 720, "DCF77 im Ein-Fenster: eingeschaltet oder night_wm nicht gesetzt");
        /* AKN.4 woertlich: wie das erste TIME -- liegt ein Aus, gilt es */
        tab_a (); frisch (1); gmain.year = 2000; zeit (0, 0, 0); startblock ();
        DCF (3, 1, 0);
        pruef (! display.display_power_is_on && spd_n == 1 && strstr (logbuf, "night: start aus (23:00)\r\n") != NULL, "DCF77 im Aus-Fenster bei Display an: nicht wie das erste TIME ausgewertet");
        /* night_wm schon gesetzt: DCF77 wertet nicht erneut als Start aus */
        tab_a (); frisch (1); zeit (3, 12, 0); night_wm = 3 * 1440 + 720;
        DCF (3, 1, 0);
        pruef (display.display_power_is_on && spd_n == 0 && night_wm == 3 * 1440 + 720, "DCF77 bei gesetztem night_wm: als Start ausgewertet");
    }

    printf ("  Kategorien:\n");
    for (i = 1; i <= kat; i++) printf ("    %-28s %9lu Pruefungen, %lu FEHL\n", kat_name[i], kat_n[i], kat_f[i]);
    printf ("  Tabellen %lu, Startpunkte %lu (= Tabellen x 10'080: %s), davon mit Ereignis %lu, ohne %lu, Gleichstand Aus/Ein %lu\n",
            tabellen, mit_ereignis + ohne_ereignis, (mit_ereignis + ohne_ereignis == tabellen * 10080) ? "ja" : "NEIN",
            mit_ereignis, ohne_ereignis, gleichstand);
    pruef (mit_ereignis + ohne_ereignis == tabellen * 10080, "Zaehlung Startpunkte");
    printf ("a70: %lu Pruefungen, %lu FEHL -- %s\n", faelle, fehl, fehl ? "FEHL" : "OK");
    return fehl ? 1 : 0;
}
