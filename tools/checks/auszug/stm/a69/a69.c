/* Pruefstand A69 (Paket 2026-10-11, S1.3): Nachtfenster beim Start und bei Zeitspruengen.
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
 * Gleichstand (Aus und Ein in derselben Minute) wird gezaehlt und benannt, nicht bewertet. */
#ifdef PRUEFSTAND_SCHNELLE_TYPEN
#include "pruefstand.h"
#endif
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

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
static struct { uint_fast8_t wday, hour, minute; uint_fast16_t year; } gmain;
static struct { uint_fast8_t display_power_is_on, ambilight_power_is_on, do_display_icon; } display;
static struct { uint_fast8_t is_online; char ipaddress[16]; } esp8266;
volatile uint32_t   uptime;
#define MAX_OVERLAYS 8
static uint_fast8_t var_sync_pending, ip_ticker_pending, pending_ticker_restore, show_overlay_idx = MAX_OVERLAYS;
static uint32_t     show_icon_stop_time;
/* Ticker-Attrappe wie display_set_ticker(): bei ausgeschaltetem Display tut sie NICHTS (display.c) */
static int ticker_rest, ticker_gezeigt, ticker_laenge = 40;
static void display_set_ticker (const unsigned char * s, uint_fast8_t w) { (void) s; (void) w; if (display.display_power_is_on) { ticker_rest = ticker_laenge; ticker_gezeigt++; } }
static uint_fast8_t display_ticker_active (void) { return ticker_rest > 0; }
static uint_fast8_t display_clock_flag, dcf77_enabled;
/* Sendeprotokoll (A68): D = Display-Power, A = Ambilight-Power, in Reihenfolge */
static char proto[64];
static void var_send_display_power (void) { strcat (proto, "D"); }
static void var_send_display_ambilight_power (void) { strcat (proto, "A"); }
static void display_set_ambilight_power (uint_fast8_t on) { display.ambilight_power_is_on = on ? 1 : 0; }
static int spd_n, spd_sync;
static uint_fast8_t set_display_power_echt (uint_fast8_t, uint_fast8_t);
static uint_fast8_t set_display_power (uint_fast8_t on, uint_fast8_t sync) { spd_n++; spd_sync = sync; return set_display_power_echt (on, sync); }
#include "main-a69.c"

static unsigned long faelle, fehl, kat_n[16], kat_f[16], gleichstand, mit_ereignis, ohne_ereignis, tabellen;
static const char * kat_name[16]; static int kat;
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
static void frisch (int disp) { night_off_due = 0; ticker_rest = 0; ticker_gezeigt = 0; ip_ticker_pending = 0; pending_ticker_restore = 0; display_clock_flag = 0; gmain.year = 2026; uptime = 5; night_wm = 0xFFFF; display.display_power_is_on = disp; display.ambilight_power_is_on = 1; spd_n = 0; spd_sync = -1; logbuf[0] = 0; proto[0] = 0; esp8266.is_online = 1; memset (ambilight_night_time, 0, sizeof ambilight_night_time); }
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

    printf ("  Kategorien:\n");
    for (i = 1; i <= kat; i++) printf ("    %-28s %9lu Pruefungen, %lu FEHL\n", kat_name[i], kat_n[i], kat_f[i]);
    printf ("  Tabellen %lu, Startpunkte %lu (= Tabellen x 10'080: %s), davon mit Ereignis %lu, ohne %lu, Gleichstand Aus/Ein %lu\n",
            tabellen, mit_ereignis + ohne_ereignis, (mit_ereignis + ohne_ereignis == tabellen * 10080) ? "ja" : "NEIN",
            mit_ereignis, ohne_ereignis, gleichstand);
    pruef (mit_ereignis + ohne_ereignis == tabellen * 10080, "Zaehlung Startpunkte");
    printf ("a69: %lu Pruefungen, %lu FEHL -- %s\n", faelle, fehl, fehl ? "FEHL" : "OK");
    return fehl ? 1 : 0;
}
