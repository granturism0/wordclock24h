/* Pruefstand a80 (Paket 2026-10-13, S2.7, A80, AKC.1): Ein Icon, das bei ausgeschaltetem Display vollstaendig
 * ankommt, wird verworfen statt vorgemerkt; ein vorgemerkter Ticker-Restore wartet nicht darauf. Geht das
 * Display waehrend eines Icons aus, endet das Icon sofort, und das naechste beginnt von vorn. In keinem Fall
 * "icon_freeze: ENTER". Bei eingeschaltetem Display bleibt alles wie bisher.
 * WOERTLICH aus dem Stand (extract.py): set_display_power, Zweig ESP8266_ICONDATA, icon_freeze, Icon-Stop,
 * Restore-Bedingung, display_icon (+ display_icon_cancel). Attrappen: LEDs, Log, ESP, Ticker.
 * Ein Hauptloop-Durchlauf = 1/64 s (die Animation laeuft im Hauptloop, main.c animation_flag). */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#define TRUE  1
#define FALSE 0
#define WC_ROWS             10
#define WC_COLUMNS          11
#define DSP_DISPLAY_LEDS    (WC_ROWS * WC_COLUMNS)
#define MAX_OVERLAYS        32
#define ICON_COLORS         10
#define DISPLAY_CLOCK_FLAG_UPDATE_ALL           0x04
#define DISPLAY_CLOCK_FLAG_POWER_ON             0x08
#define DISPLAY_CLOCK_FLAG_POWER_OFF            0x10
#define NIGHT_OFF_AFTER_IP      0xFFFFFFFF

typedef struct { uint8_t red, green, blue, white; } DSP_COLORS;
typedef struct { uint8_t r, g, b, w; } LED_RGB;
#define RESET_LED_RGB(t)            do { (t).r = (t).g = (t).b = (t).w = 0; } while (0)
#define SET_DSP_RGB(t,R,G,B,W)      do { (t).red = (R); (t).green = (G); (t).blue = (B); (t).white = (W); } while (0)
#define CALC_LED_RGB(t,s)           do { (t).r = (s).red; (t).g = (s).green; (t).b = (s).blue; (t).w = (s).white; } while (0)

typedef struct
{
    uint_fast8_t    rows;
    uint_fast8_t    cols;
    char            colors[WC_ROWS * WC_COLUMNS];
    char            animation_on[WC_ROWS * WC_COLUMNS];
    char            animation_off[WC_ROWS * WC_COLUMNS];
    uint_fast8_t    duration;
} DISPLAY_ICON;

static struct
{
    uint_fast8_t    display_power_is_on;
    uint_fast8_t    ambilight_power_is_on;
    uint_fast8_t    do_display_icon;
    uint_fast8_t    animation_start_flag;
    uint_fast8_t    animation_stop_flag;
    uint_fast8_t    display_brightness;
    uint_fast8_t    dimmed_display_colors[16];
} display;

static struct { uint8_t state[DSP_DISPLAY_LEDS]; } led;
static DISPLAY_ICON     display_icon_st;
static const DSP_COLORS icon_colors[ICON_COLORS];

/* Log: alles in einen Puffer, ausgewertet je Szenario */
static char             logbuf[16384];
static void log_uart_puts (const char * s) { if (strlen (logbuf) + strlen (s) < sizeof logbuf) strcat (logbuf, s); }
static void log_printf (const char * f, ...) { char b[256]; va_list a; va_start (a, f); vsnprintf (b, sizeof b, f, a); va_end (a); log_uart_puts (b); }
#define log_message(s)          do { log_uart_puts (s); log_uart_puts ("\r\n"); } while (0)
#define debug_log_message(s)
#define debug_log_printf(...)

static void display_set_display_led (uint_fast16_t i, LED_RGB * p, uint_fast8_t r) { (void) i; (void) p; (void) r; }
static void display_refresh_display_leds (void) { }
static void display_dim_display_dsp_colors (DSP_COLORS * t, DSP_COLORS * s, uint_fast16_t b, uint_fast8_t r) { (void) b; (void) r; *t = *s; }
static void display_set_ambilight_power (uint_fast8_t on) { display.ambilight_power_is_on = on; }
static void var_send_display_power (void) { }
static void var_send_display_ambilight_power (void) { }
static struct { uint_fast8_t is_online; } esp8266 = { 1 };
static uint_fast8_t     dcf77_enabled;
static void dcf77_update (void) { dcf77_enabled = ! display.display_power_is_on && ! display.ambilight_power_is_on; }   /* A82: wie main.c */
static uint32_t         night_off_due;
static void night_off_apply (void) { night_off_due = 0; }
static uint_fast8_t     ticker_aktiv;
static uint_fast8_t display_ticker_active (void) { return ticker_aktiv; }
static uint_fast8_t display_read_icon (void) { return 2; }      /* Uebertragung vollstaendig */

static uint32_t         local_uptime;
static uint32_t         show_icon_stop_time;
static uint_fast8_t     icon_duration;
static uint_fast8_t     show_overlay_idx = MAX_OVERLAYS;
static uint_fast8_t     pending_ticker_restore;
static uint_fast8_t     display_clock_flag;

#include "auszug-display.c"
#include "auszug-main.c"

static unsigned long    tick;           /* Durchlaeufe zu je 1/64 s */

static void
loop_once (void)
{
    local_uptime = tick / 64;
    pf_icon_freeze ();
    pf_icon_stop ();
    pf_restore ();
    if (display_clock_flag) display_clock_flag = 0;          /* display_clock() verbraucht das Flag */
    if (! display_ticker_active () && display.do_display_icon) display_icon ();   /* display_animation() */
    tick++;
}
static void laufe (unsigned long n) { while (n--) loop_once (); }
static void power (uint_fast8_t on) { display_clock_flag = set_display_power (on, TRUE); }
static void icon_kommt (uint_fast8_t dauer)
{
    display_icon_st.rows = 5; display_icon_st.cols = 5; display_icon_st.duration = dauer;
    memset (display_icon_st.colors, '1', 25); display_icon_st.colors[25] = 0;
    display_icon_st.animation_on[0] = 0; display_icon_st.animation_off[0] = 0;
    icon_duration = dauer;
    pf_icondata ();
}
static int zaehle (const char * s) { int n = 0; const char * p = logbuf; while ((p = strstr (p, s))) { n++; p += strlen (s); } return n; }

static int fehler;
static void pruefe (int ok, const char * was) { printf ("  %s  %s\n", ok ? "OK  " : "FEHL", was); if (! ok) fehler++; }
static void neu (void) { logbuf[0] = 0; tick = 64 * 1000; pending_ticker_restore = 0; show_icon_stop_time = 0; ticker_aktiv = 0;
                         display.do_display_icon = 0; display.display_power_is_on = 1; display.animation_stop_flag = 1;
                         display.display_brightness = 15; memset (display.dimmed_display_colors, 31, 16); }

int
main (void)
{
    /* Fall 1: Display aus, Ticker durch und Restore vorgemerkt, dann kommt ein Icon */
    printf ("Fall 1: Icon kommt bei ausgeschaltetem Display\n");
    neu (); power (0); laufe (2);
    pending_ticker_restore = 1;
    { uint32_t vorher = show_icon_stop_time; icon_kommt (10);
      pruefe (display.do_display_icon == 0, "do_display_icon bleibt 0");
      pruefe (show_icon_stop_time == vorher, "show_icon_stop_time unveraendert"); }
    pruefe (zaehle ("icon: verworfen, display aus\r\n") == 1, "genau eine Zeile \"icon: verworfen, display aus\"");
    laufe (3);
    pruefe (pending_ticker_restore == 0, "Restore laeuft sofort, wartet nicht auf das Einschalten");
    power (1); laufe (64 * 20);
    pruefe (zaehle ("display_icon: start") == 0, "nach dem Einschalten erscheint das verworfene Icon nicht");
    pruefe (zaehle ("icon_freeze: ENTER") == 0, "kein icon_freeze: ENTER");

    /* Fall 2: Display an -- unveraendert */
    printf ("Fall 2: Icon bei eingeschaltetem Display\n");
    neu (); power (1); laufe (2);
    pending_ticker_restore = 1;
    icon_kommt (8);
    pruefe (display.do_display_icon == 1, "do_display_icon = 1");
    pruefe (show_icon_stop_time == local_uptime + 8, "show_icon_stop_time = uptime + Dauer");
    pruefe (zaehle ("icon: verworfen") == 0, "keine Verwerfungszeile");
    laufe (64 * 3);
    pruefe (pending_ticker_restore == 1, "Restore wartet, solange das Icon laeuft");
    pruefe (zaehle ("display_icon: start") == 1, "Icon gestartet");
    laufe (64 * 8);
    pruefe (zaehle ("display_icon: finished") == 1 && display.do_display_icon == 0, "Icon regulaer beendet");
    pruefe (show_icon_stop_time == 0 && pending_ticker_restore == 0, "Stop-Timer abgelaufen, Restore danach ausgefuehrt");
    pruefe (zaehle ("icon_freeze: ENTER") == 0, "kein icon_freeze: ENTER");

    /* Fall 3: Icon laeuft, Display geht aus (Nachtfenster) */
    printf ("Fall 3: Display geht waehrend eines Icons aus\n");
    neu (); power (1); laufe (2);
    pending_ticker_restore = 1;
    icon_kommt (20);
    laufe (64 * 3);
    pruefe (zaehle ("display_icon: start") == 1 && display.do_display_icon == 1, "Icon laeuft");
    power (0);
    pruefe (display.do_display_icon == 0, "Icon endet sofort mit dem Aus");
    laufe (64 * 18);
    pruefe (pending_ticker_restore == 0, "Restore nach Ablauf des Stop-Timers, ohne Einschalten");
    pruefe (zaehle ("icon_freeze: ENTER") == 0, "kein icon_freeze: ENTER");
    power (1); laufe (64 * 2);
    pruefe (zaehle ("display_icon: start") == 1, "abgebrochenes Icon setzt nach dem Einschalten nicht fort");
    icon_kommt (6); laufe (2);
    pruefe (zaehle ("display_icon: start") == 2, "naechstes Icon beginnt von vorn (Zustand CLEAR_DISPLAY)");
    laufe (64 * 7);
    pruefe (zaehle ("display_icon: finished") == 1 && display.do_display_icon == 0, "naechstes Icon laeuft vollstaendig ab");

    printf ("a80: %s (%d Fehler)\n", fehler ? "FEHL" : "OK", fehler);
    return fehler ? 1 : 0;
}
