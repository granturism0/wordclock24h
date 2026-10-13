/* Pruefstand a81 (Paket 2026-10-13, S2.5, A81, AKA.4): Die Marker-Weissfarbe des Ambilights kommt nach einem
 * STM-Neustart von ihrem eigenen Platz (EEPROM_DATA_OFFSET_AMBI_MARKER_W_COLOR), unabhaengig von der
 * Ambilight-Weissfarbe; die gelesene Marker-Farbe ist auf 0..MAX_COLOR_STEPS-1 geklemmt (sie indiziert
 * pwmtable8[MAX_COLOR_STEPS]); EEPROMs aus Layoutversionen vor 2.9 (ohne den Platz) ergeben einen definierten Wert.
 * Attrappen wie a56: EEPROM-Abbild 4096 Byte hinter dem ECHTEN eeprom_write(); Start nach main.c modelliert
 * (display_init-Startwert, Lesen nur bei gueltiger Version, upgrade_eep_version() bei Version != EEPROM_VERSION).
 * Uebersetzt mit DSP_USE_SK6812_RGBW=1 (RGBW) und =0 (nur RGB). */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "layout.h"

#define TRUE  1
#define FALSE 0
#ifndef DSP_USE_SK6812_RGBW
#define DSP_USE_SK6812_RGBW         1
#endif
#define ANIMATION_MODES             20
#define COLOR_ANIMATION_MODES       3
#define AMBILIGHT_MODES             5
#define AMBILIGHT_MODE_NORMAL       0
#define AMBILIGHT_MODE_CLOCK        1
#define AMBILIGHT_FLAG_SECONDS_MARKER 0x01
#define ANIMATION_FLAG_FAVOURITE    0x01
#define MAX_COLOR_STEPS             64
#define DSP_AMBILIGHT_LEDS          60
#define MAX_TICKER_DECELERATION     15
#define DEFAULT_TICKER_DECELERATION 3
#define DISPLAY_FLAGS_PERMANENT_IT_IS     0x01
#define DISPLAY_FLAGS_SYNC_AMBILIGHT      0x02
#define DISPLAY_FLAGS_SYNC_CLOCK_MARKERS  0x04
/* Gegenprobe gegen einen Stand vor A56: der Platz existiert dort nicht, die Lesefunktion fragt ihn nicht ab */
#ifndef EEPROM_DATA_OFFSET_AMBI_POWER
#define EEPROM_DATA_OFFSET_AMBI_POWER 1979
#define EEPROM_DATA_SIZE_AMBI_POWER   2
#define EEPROM_AMBI_POWER_MARK        0xA5
#endif

#if DSP_USE_SK6812_RGBW == 1
typedef struct { uint8_t red, green, blue, white; } DSP_COLORS;
#define COPY_DSP_RGB_SAFE(t,sp) do { \
    (t).red   = ((sp)->red   < MAX_COLOR_STEPS) ? (sp)->red   : MAX_COLOR_STEPS - 1; \
    (t).green = ((sp)->green < MAX_COLOR_STEPS) ? (sp)->green : MAX_COLOR_STEPS - 1; \
    (t).blue  = ((sp)->blue  < MAX_COLOR_STEPS) ? (sp)->blue  : MAX_COLOR_STEPS - 1; \
    (t).white = ((sp)->white < MAX_COLOR_STEPS) ? (sp)->white : MAX_COLOR_STEPS - 1; } while (0)
#define WEISS(c) ((int) (c).white)
#else
typedef struct { uint8_t red, green, blue; } DSP_COLORS;
#define COPY_DSP_RGB_SAFE(t,sp) do { \
    (t).red   = ((sp)->red   < MAX_COLOR_STEPS) ? (sp)->red   : MAX_COLOR_STEPS - 1; \
    (t).green = ((sp)->green < MAX_COLOR_STEPS) ? (sp)->green : MAX_COLOR_STEPS - 1; \
    (t).blue  = ((sp)->blue  < MAX_COLOR_STEPS) ? (sp)->blue  : MAX_COLOR_STEPS - 1; } while (0)
#define WEISS(c) 0
#endif
#define COPY_DSP_RGB(t,s)  do { (t) = (s); } while (0)
typedef struct { uint_fast8_t deceleration, flags; } MODE;
static struct {
    uint_fast8_t display_mode, ambilight_mode, display_flags, animation_mode, color_animation_mode;
    DSP_COLORS display_colors, ambilight_colors, ambilight_marker_colors, saved_display_colors, saved_ambilight_colors;
    uint_fast8_t display_brightness, ambilight_brightness, automatic_brightness, saved_ambilight_brightness;
    uint_fast8_t ambilight_leds, ambilight_led_offset, ticker_deceleration;
    uint_fast8_t display_power_is_on, ambilight_power_is_on;
    MODE animations[ANIMATION_MODES], color_animations[COLOR_ANIMATION_MODES], ambilight_modes[AMBILIGHT_MODES];
    uint_fast8_t dimmed_display_colors[MAX_BRIGHTNESS + 1], dimmed_ambilight_colors[MAX_BRIGHTNESS + 1];
    char date_ticker_format[DATE_TICKER_FORMAT_LEN];
} display;

/* EEPROM-Attrappe */
static uint8_t img[4096];
static int zyklen;
#define I2C_OK 0
static uint_fast8_t eeprom_is_up = 1, eeprom_addr = 0xA0;
static int i2c_read (int dev, uint_fast16_t a, int al, uint8_t * b, uint_fast16_t n) { (void) dev; (void) al; memcpy (b, img + (a % 4096), n); return I2C_OK; }
static int i2c_write (int dev, uint_fast16_t a, int al, uint8_t * b, uint_fast16_t n) { (void) dev; (void) al; memcpy (img + a, b, n); return I2C_OK; }
static void eeprom_waitstates (void) { zyklen++; }
static unsigned esp8266_uart_rxdrops (void) { return 0; }
static uint32_t diag_ticks (void) { return 0; }
static const uint32_t diag_ticks_per_ms = 1;
static void log_printf (const char * f, ...) { (void) f; }
static uint_fast8_t eep_read (uint_fast16_t a, uint8_t * b, uint_fast16_t n) { memcpy (b, img + a, n); return 1; }
#define eep_write(a,b,l) eeprom_write (a, b, l)
#define eep_is_up        eeprom_is_up
uint_fast8_t eeprom_write (uint_fast16_t start_addr, uint8_t * buffer, uint_fast16_t cnt);

/* Anzeige-Attrappen; die Marker-Dimmung prueft, ob die Farbe als Index in pwmtable8[MAX_COLOR_STEPS] taugt */
static int index_ausserhalb;
static void display_refresh_ambilight_leds (void) { }
static void display_show_ambilight_off (uint_fast8_t r) { (void) r; }
static void display_calc_dimmed_ambilight_colors (void) { }
static void display_calc_dimmed_ambilight_marker_colors (void)
{
    DSP_COLORS * c = &display.ambilight_marker_colors;
    if (c->red >= MAX_COLOR_STEPS || c->green >= MAX_COLOR_STEPS || c->blue >= MAX_COLOR_STEPS || WEISS (*c) >= MAX_COLOR_STEPS) index_ausserhalb++;
}
static void display_calc_dimmed_display_colors (void) { }
static void display_init_color_animation (void) { }
static void display_init_ambilight_mode (void) { }
#define S(n) static void n (void) { }
S(display_save_display_colors) S(display_save_display_mode) S(display_save_display_flags) S(display_save_animation_mode)
S(display_save_color_animation_mode) S(display_save_brightness) S(display_save_automatic_brightness)
S(display_save_ambilight_brightness)
S(display_save_ambilight_mode) S(display_save_number_of_ambilight_leds) S(display_save_ambilight_led_offset)
S(display_save_animations) S(display_save_color_animations) S(display_save_ambilight_mode_decelerations)
S(display_save_dimmed_display_colors) S(display_save_dimmed_ambilight_colors) S(display_save_ticker_deceleration)
S(display_save_date_ticker_format)

#include "auszug.c"

static unsigned long faelle, fehl; static char was[300];
static void pruef (int ok, const char * w) { faelle++; if (! ok) { fehl++; if (fehl <= 12) printf ("  FEHL  %s\n", w); } }
static void ver (uint32_t v) { memcpy (img, &v, 4); }
static const DSP_COLORS INIT_MARKER = { 0, MAX_COLOR_STEPS / 2, MAX_COLOR_STEPS / 2 };    /* display_init(): Cyan */

static void start (void)
{
    uint32_t v; memcpy (&v, img, 4);
    zyklen = 0; index_ausserhalb = 0;
    display.ambilight_marker_colors = INIT_MARKER;
    display.ambilight_power_is_on = 1; display.display_power_is_on = 1;
    if (eep_is_up && (v & 0xFF0000FF) == 0 && v >= EEPROM_VERSION_1_5 && v <= EEPROM_VERSION)
        display_read_config_from_eep (v);
    if (eep_is_up && v != EEPROM_VERSION)
    {
        ver (EEPROM_VERSION);
        display_write_config_to_eep ();
    }
}
static void zufall (uint32_t v) { int i; for (i = 0; i < 4096; i++) img[i] = (uint8_t) rand (); ver (v); }
static int klemm (int x) { return x >= MAX_COLOR_STEPS ? MAX_COLOR_STEPS - 1 : x; }
static int gleich (DSP_COLORS a, DSP_COLORS b) { return a.red == b.red && a.green == b.green && a.blue == b.blue && WEISS (a) == WEISS (b); }
#define MW  EEPROM_DATA_OFFSET_AMBI_MARKER_W_COLOR
#define AW  EEPROM_DATA_OFFSET_AMBI_W_COLOR
#define MC  EEPROM_DATA_OFFSET_AMBI_MARKER_COLORS

int main (void)
{
    int x, y, v, k;
    srand (81);

    /* Layout: die beiden Weiss-Plaetze sind verschieden */
    snprintf (was, sizeof was, "Layout: Marker-Weiss %d, Ambilight-Weiss %d", (int) MW, (int) AW);
    pruef (MW != AW && EEPROM_DATA_SIZE_AMBI_MARKER_W_COLOR == 1 && EEPROM_DATA_SIZE_AMBI_W_COLOR == 1, was);

    /* 1: Marker-Weiss X und Ambilight-Weiss Y ueber den echten Setter bzw. Speicherpfad schreiben, neu starten:
     *    beide kommen getrennt zurueck (alle 64 x 64 Paare), der Marker samt RGB */
    for (x = 0; x < MAX_COLOR_STEPS; x++) for (y = 0; y < MAX_COLOR_STEPS; y++)
    {
        DSP_COLORS m = { (uint8_t) ((x * 7) & 63), (uint8_t) ((y * 5) & 63), (uint8_t) ((x + y) & 63) };
#if DSP_USE_SK6812_RGBW == 1
        m.white = (uint8_t) x;
#endif
        zufall (EEPROM_VERSION); start ();
        display_set_ambilight_marker_colors (&m);
#if DSP_USE_SK6812_RGBW == 1
        display.ambilight_colors.white = (uint8_t) y;
#endif
        display_save_ambilight_colors ();
        start ();
        snprintf (was, sizeof was, "geschrieben Marker %d/%d/%d W %d, Ambilight-W %d; nach Neustart Marker %d/%d/%d W %d, Ambilight-W %d",
                  m.red, m.green, m.blue, WEISS (m), DSP_USE_SK6812_RGBW ? y : 0, display.ambilight_marker_colors.red, display.ambilight_marker_colors.green,
                  display.ambilight_marker_colors.blue, WEISS (display.ambilight_marker_colors), WEISS (display.ambilight_colors));
        pruef (gleich (display.ambilight_marker_colors, m) && WEISS (display.ambilight_colors) == (DSP_USE_SK6812_RGBW ? y : 0) && zyklen == 0, was);
    }

    /* 2: rohe Belegung jedes Marker-Kanals (alle 256 Werte) bei Layout 2.9: gelesen wird min (v, 63);
     *    die Dimmung sieht nie einen Index ausserhalb pwmtable8[] */
    for (k = 0; k < 4; k++) for (v = 0; v < 256; v++)
    {
        int adr = k < 3 ? MC + k : MW, soll;
        uint8_t ist;
        if (k == 3 && ! DSP_USE_SK6812_RGBW) break;
        zufall (EEPROM_VERSION);
        img[MC] = img[MC + 1] = img[MC + 2] = img[MW] = 10; img[adr] = (uint8_t) v;
        img[AW] = (uint8_t) (v ^ 0x2A);                         /* Ambilight-Weiss absichtlich anders */
        start ();
        ist = k == 0 ? display.ambilight_marker_colors.red : k == 1 ? display.ambilight_marker_colors.green
            : k == 2 ? display.ambilight_marker_colors.blue : (uint8_t) WEISS (display.ambilight_marker_colors);
        soll = klemm (v);
        snprintf (was, sizeof was, "Kanal %d roh %d: gelesen %d statt %d, Index ausserhalb %d", k, v, ist, soll, index_ausserhalb);
        pruef (ist == soll && index_ausserhalb == 0, was);
    }

    /* 3: Layoutversionen vor 2.9 (der Platz existiert dort nicht, Inhalt beliebig): Marker-Farbe unabhaengig
     *    vom Inhalt, im Bereich, nach dem Upgrade auf ihrem Platz geschrieben und beim naechsten Start gleich */
    {
        static const uint32_t V[] = { 0x00010500, 0x00010600, 0x00010700, 0x00010800, 0x00010900, 0x00020000, 0x00020100,
                                      0x00020200, 0x00020300, 0x00020400, 0x00020500, 0x00020600, 0x00020700, 0x00020800 };
        unsigned i; int r; DSP_COLORS erst = { 0 }, nun; int erst_da = 0;
        for (i = 0; i < sizeof V / sizeof V[0]; i++) for (r = 0; r < 64; r++)
        {
            zufall (V[i]);
            if (r & 1) memset (img + 4, 0xFF, 4092);           /* auch: nie beschriebenes EEPROM-Gebiet */
            ver (V[i]);
            start (); nun = display.ambilight_marker_colors;
            if (! erst_da) { erst = nun; erst_da = 1; }
            snprintf (was, sizeof was, "alt 0x%08x (%d): Marker %d/%d/%d W %d, Platz danach %d, erster Fall %d/%d/%d W %d, Index ausserhalb %d",
                      (unsigned) V[i], r, nun.red, nun.green, nun.blue, WEISS (nun), img[MW], erst.red, erst.green, erst.blue, WEISS (erst), index_ausserhalb);
            pruef (gleich (nun, erst) && index_ausserhalb == 0 && (! DSP_USE_SK6812_RGBW || img[MW] == WEISS (nun))
                   && img[MC] == nun.red && img[MC + 1] == nun.green && img[MC + 2] == nun.blue, was);
            start ();
            snprintf (was, sizeof was, "alt 0x%08x (%d), zweiter Start: Marker %d/%d/%d W %d", (unsigned) V[i], r,
                      display.ambilight_marker_colors.red, display.ambilight_marker_colors.green, display.ambilight_marker_colors.blue, WEISS (display.ambilight_marker_colors));
            pruef (gleich (display.ambilight_marker_colors, nun) && zyklen == 0, was);
        }
        printf ("  alte Layouts: Marker nach Upgrade %d/%d/%d W %d\n", erst.red, erst.green, erst.blue, WEISS (erst));
    }

    /* 4: frisches EEPROM (alles 0xFF, Version ungueltig): display_init-Wert bleibt, wird geschrieben, kommt zurueck */
    memset (img, 0xFF, sizeof img); start ();
    snprintf (was, sizeof was, "frisch: Marker %d/%d/%d W %d, Platz %d", display.ambilight_marker_colors.red, display.ambilight_marker_colors.green,
              display.ambilight_marker_colors.blue, WEISS (display.ambilight_marker_colors), img[MW]);
    pruef (gleich (display.ambilight_marker_colors, INIT_MARKER) && (! DSP_USE_SK6812_RGBW || img[MW] == WEISS (INIT_MARKER)), was);
    start ();
    pruef (gleich (display.ambilight_marker_colors, INIT_MARKER) && zyklen == 0, "frisch, zweiter Start");

    printf ("a81: RGBW %d, Marker-Weiss @%d, Ambilight-Weiss @%d; %lu Pruefungen, %lu FEHL -- %s\n",
            DSP_USE_SK6812_RGBW, (int) MW, (int) AW, faelle, fehl, fehl ? "FEHL" : "OK");
    return fehl ? 1 : 0;
}
