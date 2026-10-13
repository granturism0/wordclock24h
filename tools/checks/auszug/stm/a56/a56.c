/* Pruefstand a56 (Paket 2026-10-13, S2.3, A56, AKA.1/AKA.2): Ambilight-Power uebersteht einen STM-Neustart.
 * Zwei Byte hinter dem Layoutende (Marke 0xA5, Wert 0/1), ohne Layoutversion (Ent-9).
 * Attrappen: EEPROM-Abbild 4096 Byte (AT24C32) hinter dem ECHTEN eeprom_write() -- I2C liest/schreibt das
 * Abbild, jeder Wartezyklus ist ein Seitenschreiben. Der Start wird nach main.c modelliert:
 * display_init-Startwert, read_configuration_from_eep() nur bei gueltiger Version (1.5..EEPROM_VERSION, MSB/LSB 0),
 * upgrade_eep_version() schreibt display_write_config_to_eep() bei Version != EEPROM_VERSION. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "layout.h"

#define TRUE  1
#define FALSE 0
#define DSP_USE_SK6812_RGBW         1
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

/* Referenz aus dem Stand vor A56 (3.2.32): Layoutende 1979, Version 2.9. Fehlen die neuen Konstanten
 * (Gegenprobe), gelten diese Werte -- dann pruefen die Faelle das Verhalten des alten Codes. */
#define ALT_LAYOUT_ENDE             1979
#define ALT_EEPROM_VERSION          0x00020900
#ifndef EEPROM_DATA_OFFSET_AMBI_POWER
#define EEPROM_DATA_OFFSET_AMBI_POWER 1979
#define EEPROM_DATA_SIZE_AMBI_POWER   2
#endif
#ifndef EEPROM_AMBI_POWER_MARK
#define EEPROM_AMBI_POWER_MARK      0xA5
#endif

typedef struct { uint8_t red, green, blue, white; } DSP_RGB;
#define COPY_DSP_RGB(t,s)  do { (t) = (s); } while (0)
typedef struct { uint_fast8_t deceleration, flags; } MODE;
static struct {
    uint_fast8_t display_mode, ambilight_mode, display_flags, animation_mode, color_animation_mode;
    DSP_RGB display_colors, ambilight_colors, ambilight_marker_colors, saved_display_colors, saved_ambilight_colors;
    uint_fast8_t display_brightness, ambilight_brightness, automatic_brightness, saved_ambilight_brightness;
    uint_fast8_t ambilight_leds, ambilight_led_offset, ticker_deceleration;
    uint_fast8_t display_power_is_on, ambilight_power_is_on;
    MODE animations[ANIMATION_MODES], color_animations[COLOR_ANIMATION_MODES], ambilight_modes[AMBILIGHT_MODES];
    uint_fast8_t dimmed_display_colors[MAX_BRIGHTNESS + 1], dimmed_ambilight_colors[MAX_BRIGHTNESS + 1];
    char date_ticker_format[DATE_TICKER_FORMAT_LEN];
} display;

/* EEPROM-Attrappe */
static uint8_t img[4096];
static int zyklen, n_lesen, schreib_lo, schreib_hi;
#define I2C_OK 0
static uint_fast8_t eeprom_is_up = 1, eeprom_addr = 0xA0;
static int i2c_read (int dev, uint_fast16_t a, int al, uint8_t * b, uint_fast16_t n) { (void) dev; (void) al; memcpy (b, img + (a % 4096), n); return I2C_OK; }
static int i2c_write (int dev, uint_fast16_t a, int al, uint8_t * b, uint_fast16_t n)
{ (void) dev; (void) al; memcpy (img + a, b, n); if ((int) a < schreib_lo) schreib_lo = a; if ((int) (a + n) > schreib_hi) schreib_hi = a + n; return I2C_OK; }
static void eeprom_waitstates (void) { zyklen++; }
static unsigned esp8266_uart_rxdrops (void) { return 0; }
static uint32_t diag_ticks (void) { return 0; }
static const uint32_t diag_ticks_per_ms = 1;
static void log_printf (const char * f, ...) { (void) f; }
static uint_fast8_t eep_read (uint_fast16_t a, uint8_t * b, uint_fast16_t n) { n_lesen++; memcpy (b, img + a, n); return 1; }
#define eep_write(a,b,l) eeprom_write (a, b, l)
#define eep_is_up        eeprom_is_up
uint_fast8_t eeprom_write (uint_fast16_t start_addr, uint8_t * buffer, uint_fast16_t cnt);

/* Anzeige-Attrappen: die beiden Ambilight-Ausgabepfade der Lesefunktion merken sich, was sie sehen */
static int led_an_gesehen;       /* Ambilight-LED-Ausgabe mit Power an */
static void display_refresh_ambilight_leds (void) { }
static void display_show_ambilight_off (uint_fast8_t r) { (void) r; }
static void display_calc_dimmed_ambilight_colors (void) { if (display.ambilight_power_is_on) led_an_gesehen++; }
static void display_calc_dimmed_ambilight_marker_colors (void) { }
static void display_calc_dimmed_display_colors (void) { }
static void display_init_color_animation (void) { }
static void display_init_ambilight_mode (void) { if (display.ambilight_power_is_on) led_an_gesehen++; }
#define S(n) static void n (void) { }
S(display_save_display_colors) S(display_save_display_mode) S(display_save_display_flags) S(display_save_animation_mode)
S(display_save_color_animation_mode) S(display_save_brightness) S(display_save_automatic_brightness)
S(display_save_ambilight_colors) S(display_save_ambilight_marker_colors) S(display_save_ambilight_brightness)
S(display_save_ambilight_mode) S(display_save_number_of_ambilight_leds) S(display_save_ambilight_led_offset)
S(display_save_animations) S(display_save_color_animations) S(display_save_ambilight_mode_decelerations)
S(display_save_dimmed_display_colors) S(display_save_dimmed_ambilight_colors) S(display_save_ticker_deceleration)
S(display_save_date_ticker_format)

#include "auszug.c"

static unsigned long faelle, fehl; static char was[300];
static void pruef (int ok, const char * w) { faelle++; if (! ok) { fehl++; if (fehl <= 12) printf ("  FEHL  %s\n", w); } }
static void ver (uint32_t v) { memcpy (img, &v, 4); }          /* Version wie read_version_from_eep: 4 Byte */
static void zaehler (void) { zyklen = 0; n_lesen = 0; schreib_lo = 1 << 30; schreib_hi = -1; led_an_gesehen = 0; }

/* Start nach main.c (Modell, siehe Kopf) */
static void start (void)
{
    uint32_t v; memcpy (&v, img, 4);
    zaehler ();
    display.ambilight_power_is_on = INIT_AMBI_POWER;
    display.display_power_is_on = 1;
    if (eep_is_up && (v & 0xFF0000FF) == 0 && v >= EEPROM_VERSION_1_5 && v <= EEPROM_VERSION)
        display_read_config_from_eep (v);
    if (eep_is_up && v != EEPROM_VERSION)
    {
        v = EEPROM_VERSION; ver (v);
        display_write_config_to_eep ();
    }
}
static void layout (uint32_t v, int b0, int b1)        /* zufaelliges Layout, Version, die beiden Byte */
{
    int i; for (i = 0; i < 4096; i++) img[i] = (uint8_t) rand ();
    ver (v); img[EEPROM_DATA_OFFSET_AMBI_POWER] = (uint8_t) b0; img[EEPROM_DATA_OFFSET_AMBI_POWER + 1] = (uint8_t) b1;
}

int main (void)
{
    static uint8_t vorher[4096];
    int b0, b1, n_aus = 0, n_an = 0;
    srand (56);

    /* Layout: Platz hinter dem alten Ende, im AT24C32, Version nicht angehoben (Ent-9) */
    snprintf (was, sizeof was, "Platz: Offset %d, Ende %d, altes Ende %d, EEPROM 4096", (int) EEPROM_DATA_OFFSET_AMBI_POWER, (int) EEPROM_DATA_END, ALT_LAYOUT_ENDE);
    pruef (EEPROM_DATA_OFFSET_AMBI_POWER >= ALT_LAYOUT_ENDE && EEPROM_DATA_OFFSET_AMBI_POWER + EEPROM_DATA_SIZE_AMBI_POWER <= EEPROM_DATA_END
           && EEPROM_DATA_END <= 4096 && EEPROM_DATA_SIZE_AMBI_POWER == 2, was);
    snprintf (was, sizeof was, "Layoutversion 0x%08x statt 0x%08x (Ent-9: ohne Anhebung)", (unsigned) EEPROM_VERSION, ALT_EEPROM_VERSION);
    pruef (EEPROM_VERSION == ALT_EEPROM_VERSION, was);
    snprintf (was, sizeof was, "Seite: 2 Byte ab %d ueberqueren eine %d-Byte-Seitengrenze", (int) EEPROM_DATA_OFFSET_AMBI_POWER, EEPROM_PAGE_SIZE);
    pruef (EEPROM_DATA_OFFSET_AMBI_POWER / EEPROM_PAGE_SIZE == (EEPROM_DATA_OFFSET_AMBI_POWER + 1) / EEPROM_PAGE_SIZE, was);

    /* 1: frisches EEPROM (alles 0xFF) => an, danach steht A5 01 da */
    memset (img, 0xFF, sizeof img); start ();
    snprintf (was, sizeof was, "frisch (0xFF): power %d, Abbild %02X %02X", (int) display.ambilight_power_is_on, img[EEPROM_DATA_OFFSET_AMBI_POWER], img[EEPROM_DATA_OFFSET_AMBI_POWER + 1]);
    pruef (display.ambilight_power_is_on == 1 && img[EEPROM_DATA_OFFSET_AMBI_POWER] == EEPROM_AMBI_POWER_MARK && img[EEPROM_DATA_OFFSET_AMBI_POWER + 1] == 1, was);
    zaehler (); start ();
    snprintf (was, sizeof was, "frisch, zweiter Start: power %d, Schreibzyklen %d", (int) display.ambilight_power_is_on, zyklen);
    pruef (display.ambilight_power_is_on == 1 && zyklen == 0, was);

    /* 2: EEPROM-Reset (Version 0) bei gespeichertem Aus => an, und A5 01 geschrieben */
    layout (0, EEPROM_AMBI_POWER_MARK, 0); start ();
    snprintf (was, sizeof was, "EEPROM-Reset bei A5 00: power %d, Abbild %02X %02X", (int) display.ambilight_power_is_on, img[EEPROM_DATA_OFFSET_AMBI_POWER], img[EEPROM_DATA_OFFSET_AMBI_POWER + 1]);
    pruef (display.ambilight_power_is_on == 1 && img[EEPROM_DATA_OFFSET_AMBI_POWER] == EEPROM_AMBI_POWER_MARK && img[EEPROM_DATA_OFFSET_AMBI_POWER + 1] == 1, was);

    /* 3: alle 65536 Belegungen der beiden Byte bei gueltigem Layout 2.9: aus genau bei A5 00, sonst an;
     *    beim Start kein Schreibzyklus, bei Aus keine Ambilight-LED-Ausgabe */
    for (b0 = 0; b0 < 256; b0++) for (b1 = 0; b1 < 256; b1++)
    {
        int soll = ! (b0 == EEPROM_AMBI_POWER_MARK && b1 == 0);
        layout (EEPROM_VERSION, b0, b1); start ();
        soll ? n_an++ : n_aus++;
        snprintf (was, sizeof was, "Start bei %02X %02X: power %d statt %d, Schreibzyklen %d, LED an gesehen %d",
                  b0, b1, (int) display.ambilight_power_is_on, soll, zyklen, led_an_gesehen);
        pruef (display.ambilight_power_is_on == (uint_fast8_t) soll && zyklen == 0 && (soll || led_an_gesehen == 0), was);
    }

    /* 4: unabhaengig von eep_version: alle gueltigen Versionen 1.5..2.9 mit A5 00 => aus, nach dem Upgrade bleibt A5 00 */
    {
        static const uint32_t V[] = { 0x00010500, 0x00010600, 0x00010700, 0x00010800, 0x00010900, 0x00020000, 0x00020100,
                                      0x00020200, 0x00020300, 0x00020400, 0x00020500, 0x00020600, 0x00020700, 0x00020800, 0x00020900 };
        unsigned i;
        for (i = 0; i < sizeof V / sizeof V[0]; i++)
        {
            layout (V[i], EEPROM_AMBI_POWER_MARK, 0); start ();
            snprintf (was, sizeof was, "Version 0x%08x, A5 00: power %d, Abbild danach %02X %02X", (unsigned) V[i], (int) display.ambilight_power_is_on,
                      img[EEPROM_DATA_OFFSET_AMBI_POWER], img[EEPROM_DATA_OFFSET_AMBI_POWER + 1]);
            pruef (display.ambilight_power_is_on == 0 && img[EEPROM_DATA_OFFSET_AMBI_POWER] == EEPROM_AMBI_POWER_MARK && img[EEPROM_DATA_OFFSET_AMBI_POWER + 1] == 0, was);
        }
    }

    /* 5: Schreiben bei Wechsel genau ein Seitenschreiben, gleicher Wert keins; Rest byte-gleich; Neustart haelt den Wert */
    {
        static const int folge[] = { 0, 0, 1, 1, 0, 1, 0, 0 };
        unsigned i; uint_fast8_t alt;
        layout (EEPROM_VERSION, 0xFF, 0xFF); start ();
        for (i = 0; i < sizeof folge / sizeof folge[0]; i++)
        {
            int wechsel;
            alt = display.ambilight_power_is_on; wechsel = alt != (uint_fast8_t) folge[i];
            memcpy (vorher, img, sizeof img); zaehler ();
            display_set_ambilight_power ((uint_fast8_t) folge[i]);
            vorher[EEPROM_DATA_OFFSET_AMBI_POWER] = img[EEPROM_DATA_OFFSET_AMBI_POWER];
            vorher[EEPROM_DATA_OFFSET_AMBI_POWER + 1] = img[EEPROM_DATA_OFFSET_AMBI_POWER + 1];
            snprintf (was, sizeof was, "Setzen %d -> %d (Schritt %u): Schreibzyklen %d statt %d, Abbild %02X %02X, Bereich %d..%d, Rest gleich %d",
                      (int) alt, folge[i], i, zyklen, wechsel, img[EEPROM_DATA_OFFSET_AMBI_POWER], img[EEPROM_DATA_OFFSET_AMBI_POWER + 1],
                      schreib_lo, schreib_hi, ! memcmp (vorher, img, sizeof img));
            pruef (zyklen == wechsel && ! memcmp (vorher, img, sizeof img)
                   && (! wechsel || (schreib_lo >= EEPROM_DATA_OFFSET_AMBI_POWER && schreib_hi <= EEPROM_DATA_OFFSET_AMBI_POWER + 2))
                   && img[EEPROM_DATA_OFFSET_AMBI_POWER] == EEPROM_AMBI_POWER_MARK && img[EEPROM_DATA_OFFSET_AMBI_POWER + 1] == folge[i], was);
            start ();
            snprintf (was, sizeof was, "Neustart nach Setzen auf %d: power %d, Schreibzyklen %d", folge[i], (int) display.ambilight_power_is_on, zyklen);
            pruef (display.ambilight_power_is_on == (uint_fast8_t) folge[i] && zyklen == 0, was);
        }
    }

    /* 6: Rueckflash: Die neue Firmware schaltet und startet; Version und alles ausser den 2 Byte bleiben byte-gleich.
     *    Die alte Firmware (Layout bis 1979, Version 2.9) liest deshalb ihr Layout wie vorher und setzt nichts zurueck
     *    (read_configuration_from_eep: eep_version <= 2.9). Ihre Schreibzugriffe enden vor 1979, die 2 Byte bleiben,
     *    und nach dem Vorwaertsflash gilt der zuletzt gespeicherte Wert. */
    {
        uint32_t v; int i, gleich = 1;
        layout (ALT_EEPROM_VERSION, 0xFF, 0xFF); memcpy (vorher, img, sizeof img);
        start (); display_set_ambilight_power (0); start (); display_set_ambilight_power (1); display_set_ambilight_power (0); start ();
        for (i = 0; i < 4096; i++) if (i != EEPROM_DATA_OFFSET_AMBI_POWER && i != EEPROM_DATA_OFFSET_AMBI_POWER + 1 && img[i] != vorher[i]) gleich = 0;
        memcpy (&v, img, 4);
        snprintf (was, sizeof was, "Rueckflash: uebriges Abbild gleich %d, Version 0x%08x, alte Firmware liest %d", gleich, (unsigned) v,
                  (v & 0xFF0000FF) == 0 && v >= EEPROM_VERSION_1_5 && v <= ALT_EEPROM_VERSION);
        pruef (gleich && v == ALT_EEPROM_VERSION && EEPROM_DATA_OFFSET_AMBI_POWER >= ALT_LAYOUT_ENDE, was);
        for (i = 4; i < ALT_LAYOUT_ENDE; i++) img[i] = (uint8_t) rand ();     /* alte Firmware schreibt ihr Layout */
        start ();
        snprintf (was, sizeof was, "Vorwaertsflash nach Rueckflash: power %d statt 0", (int) display.ambilight_power_is_on);
        pruef (display.ambilight_power_is_on == 0, was);
    }

    /* 7: EEPROM offline => Startwert an, kein Zugriff */
    eeprom_is_up = 0; layout (EEPROM_VERSION, EEPROM_AMBI_POWER_MARK, 0); start ();
    snprintf (was, sizeof was, "EEPROM offline: power %d, Lesezugriffe %d", (int) display.ambilight_power_is_on, n_lesen);
    pruef (display.ambilight_power_is_on == 1 && n_lesen == 0, was);
    eeprom_is_up = 1;

    printf ("a56: Platz %d..%d, Marke 0x%02X, Layoutende %d; %lu Pruefungen (Startbelegungen aus %d, an %d), %lu FEHL -- %s\n",
            (int) EEPROM_DATA_OFFSET_AMBI_POWER, (int) EEPROM_DATA_OFFSET_AMBI_POWER + 1, EEPROM_AMBI_POWER_MARK, (int) EEPROM_DATA_END, faelle, n_aus, n_an, fehl, fehl ? "FEHL" : "OK");
    return fehl ? 1 : 0;
}
