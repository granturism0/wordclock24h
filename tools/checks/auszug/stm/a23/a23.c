/* Pruefstand A23 (Paket 2026-10-11, S3.4): Displayfarbe nach einer Farbanimation.
 * Die Farbfunktionen kommen WOERTLICH aus display.c (Variante SK6812-RGBW). Attrappen zaehlen
 * var_send_display_colors (mit der gesendeten Farbe), Dimmrechnung und EEPROM-Schreiben (mit Inhalt). */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define DSP_USE_SK6812_RGBW             1
#define MAX_COLOR_STEPS                 64
#define COLOR_ANIMATION_MODE_NONE       0
#define COLOR_ANIMATION_MODE_RAINBOW    1
#define COLOR_ANIMATION_MODE_DAYLIGHT   2
#define COLOR_ANIMATION_MODES           3
#define EEPROM_DATA_OFFSET_DSP_COLORS   100
#define EEPROM_DATA_SIZE_DSP_COLORS     3
#define EEPROM_DATA_OFFSET_DSP_W_COLOR  200
#define EEPROM_DATA_SIZE_DSP_W_COLOR    1
typedef struct { uint_fast8_t red, green, blue, white; } DSP_COLORS;
#define SET_DSP_RGB(t,r,g,b,w)  do { (t).red = (r); (t).green = (g); (t).blue = (b); (t).white = (w); } while (0)
#define COPY_DSP_RGB(t,s)       do { (t).red = (s).red; (t).green = (s).green; (t).blue = (s).blue; (t).white = (s).white; } while (0)
#define COPY_DSP_RGB_SAFE(t,s)  do { (t).red = (s)->red < MAX_COLOR_STEPS ? (s)->red : MAX_COLOR_STEPS - 1; (t).green = (s)->green < MAX_COLOR_STEPS ? (s)->green : MAX_COLOR_STEPS - 1; \
                                     (t).blue = (s)->blue < MAX_COLOR_STEPS ? (s)->blue : MAX_COLOR_STEPS - 1; (t).white = (s)->white < MAX_COLOR_STEPS ? (s)->white : MAX_COLOR_STEPS - 1; } while (0)
static struct { DSP_COLORS display_colors, saved_display_colors; uint_fast8_t color_animation_mode; } display;
static struct { uint_fast8_t hour; } gmain = { 13 };
static const uint8_t daylight_red[24] = { 10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33 };
static const uint8_t daylight_green[24] = { 0 }, daylight_blue[24] = { 0 };
static int n_dimm, n_send, n_mode_send, n_eep_farbe; static DSP_COLORS gesendet, eep_farbe;
static void display_calc_dimmed_display_colors (void) { n_dimm++; }
static void var_send_display_colors (void) { n_send++; gesendet = display.display_colors; }
static void var_send_color_animation_mode (void) { n_mode_send++; }
static void display_save_color_animation_mode (void) { }
static void eep_write (int off, uint8_t * b, int n)
{
    if (off == EEPROM_DATA_OFFSET_DSP_COLORS && n == 3) { eep_farbe.red = b[0]; eep_farbe.green = b[1]; eep_farbe.blue = b[2]; n_eep_farbe++; }
    if (off == EEPROM_DATA_OFFSET_DSP_W_COLOR) eep_farbe.white = b[0];
}
#include "auszug.c"

static unsigned long faelle, fehl; static char was[200];
static void pruef (int ok, const char * w) { faelle++; if (! ok) { fehl++; if (fehl <= 8) printf ("  FEHL  %s\n", w); } }
static int gleich (DSP_COLORS a, DSP_COLORS b) { return a.red == b.red && a.green == b.green && a.blue == b.blue && a.white == b.white; }
static void zaehler_null (void) { n_dimm = n_send = n_mode_send = n_eep_farbe = 0; memset (&gesendet, 0xEE, sizeof gesendet); }
/* Laden wie display_read_config_from_eep(): Farbe aus dem EEPROM, Sicherung (falls der Stand das tut, LADEN_OK), Init */
static void laden (DSP_COLORS u, int modus)
{
    memset (&display, 0, sizeof display);
    display.display_colors = u;
#if LADEN_OK
    COPY_DSP_RGB (display.saved_display_colors, display.display_colors);
#endif
    display.color_animation_mode = modus;
    display_init_color_animation ();
}
static void animieren (void)                 /* was Regenbogen/Tageslicht je Schritt tun: display_colors direkt schreiben */
{
    int i; for (i = 0; i < 17; i++) SET_DSP_RGB (display.display_colors, rand () % 64, rand () % 64, rand () % 64, 0);
}
enum { WEG_SETZEN, WEG_INKREMENT, WEG_DEKREMENT };
static void nach_null (int weg)
{
    if (weg == WEG_SETZEN) display_set_color_animation_mode (COLOR_ANIMATION_MODE_NONE, 0);       /* ESP-Befehl: do_sync = FALSE */
#ifdef HAVE_DISPLAY_INCREMENT_COLOR_ANIMATION_MODE
    else if (weg == WEG_INKREMENT) { while (display.color_animation_mode != COLOR_ANIMATION_MODES - 1) display.color_animation_mode++; display_increment_color_animation_mode (0); }
    else { display.color_animation_mode = 1; display_decrement_color_animation_mode (0); }
#endif
}
static const char * WEGNAME[] = { "Setzen", "Inkrement", "Dekrement" };

int main (void)
{
    int modus, weg, k;
    srand (20261012);
    printf ("  statisch: Sicherung beim Laden vor der Animation %s, Vorgabe ohne EEPROM %s\n", LADEN_OK ? "ja" : "NEIN", VORGABE_OK ? "ja" : "NEIN");
    pruef (LADEN_OK, "Laden: Sicherung nicht vor display_init_color_animation()");
    pruef (VORGABE_OK, "display_init(): Sicherung der Vorgabefarbe fehlt");

    for (k = 0; k < 20; k++)
    {
        DSP_COLORS u = { (uint_fast8_t) (rand () % 64), (uint_fast8_t) (rand () % 64), (uint_fast8_t) (rand () % 64), (uint_fast8_t) (rand () % 64) };
        for (modus = 1; modus <= 2; modus++)
            for (weg = 0; weg < 3; weg++)
            {
                /* 1. Animation -> Modus 0: Nutzerfarbe, genau eine Farbsendung mit ihr, Dimmrechnung, kein EEPROM */
                laden (u, COLOR_ANIMATION_MODE_NONE);
                display_set_color_animation_mode (modus, 1); animieren ();
                zaehler_null (); nach_null (weg);
                snprintf (was, sizeof was, "Modus %d -> 0 (%s): Farbe %d/%d/%d/%d statt %d/%d/%d/%d, Sendungen %d, Dimm %d, EEPROM %d",
                          modus, WEGNAME[weg], display.display_colors.red, display.display_colors.green, display.display_colors.blue, display.display_colors.white,
                          u.red, u.green, u.blue, u.white, n_send, n_dimm, n_eep_farbe);
                pruef (display.color_animation_mode == 0 && gleich (display.display_colors, u) && n_send == 1 && gleich (gesendet, u) && n_dimm >= 1 && n_eep_farbe == 0, was);

                /* 2. Start im Animationsmodus (Laden mit Modus) -> 0: ebenfalls die gespeicherte Farbe */
                laden (u, modus); animieren ();
                zaehler_null (); nach_null (weg);
                snprintf (was, sizeof was, "Start in Modus %d, dann 0 (%s): Nutzerfarbe nicht zurueck", modus, WEGNAME[weg]);
                pruef (gleich (display.display_colors, u) && n_send == 1 && gleich (gesendet, u), was);

                /* 3. DC waehrend der Animation: danach die neue Farbe */
                {
                    DSP_COLORS v = { (uint_fast8_t) ((u.red + 7) % 64), (uint_fast8_t) ((u.green + 9) % 64), (uint_fast8_t) ((u.blue + 11) % 64), (uint_fast8_t) ((u.white + 13) % 64) };
                    laden (u, COLOR_ANIMATION_MODE_NONE);
                    display_set_color_animation_mode (modus, 1); animieren ();
                    zaehler_null (); display_set_display_colors (&v); animieren ();
                    pruef (n_eep_farbe == 1 && gleich (eep_farbe, v), "DC waehrend der Animation: EEPROM nicht genau einmal mit der neuen Farbe");
                    zaehler_null (); nach_null (weg);
                    snprintf (was, sizeof was, "DC waehrend Modus %d, dann 0 (%s): nicht die neue Farbe", modus, WEGNAME[weg]);
                    pruef (gleich (display.display_colors, v) && n_send == 1 && gleich (gesendet, v), was);
                }

                /* 4. IR-Schritt im Modus 0 aktualisiert die Sicherung; IR waehrend der Animation nicht */
                {
                    DSP_COLORS soll;
                    u.red = u.red > 60 ? 10 : u.red; u.green = u.green < 4 ? 30 : u.green;
                    laden (u, COLOR_ANIMATION_MODE_NONE);
                    display_increment_display_color_red (1); display_decrement_display_color_green (1);
                    soll = u; soll.red += 2; soll.green -= 2;
                    display_set_color_animation_mode (modus, 1); animieren ();
                    display_increment_display_color_blue (1);                       /* waehrend der Animation: zaehlt nicht */
                    zaehler_null (); nach_null (weg);
                    snprintf (was, sizeof was, "IR im Modus 0, dann Modus %d und zurueck (%s): %d/%d statt %d/%d", modus, WEGNAME[weg],
                              display.display_colors.red, display.display_colors.green, soll.red, soll.green);
                    pruef (gleich (display.display_colors, soll) && n_send == 1, was);
                }

                /* 5. EEPROM-Schreiben waehrend der Animation (upgrade_eep_version -> display_write_config_to_eep) */
                laden (u, modus); animieren ();
                zaehler_null (); display_save_display_colors ();
                pruef (n_eep_farbe == 1 && gleich (eep_farbe, u), "EEPROM waehrend der Animation: nicht die Nutzerfarbe");
            }
    }
    /* 6. Wechsel zwischen zwei Animationen und von 0 in eine Animation: keine Farbsendung */
    {
        DSP_COLORS u = { 5, 6, 7, 8 };
        laden (u, COLOR_ANIMATION_MODE_NONE);
        zaehler_null (); display_set_color_animation_mode (1, 1); display_set_color_animation_mode (2, 1);
        pruef (n_send == 0 && n_mode_send == 2, "0 -> Regenbogen -> Tageslicht: Farbsendung oder Modussendung falsch");
    }
    printf ("a23: %lu Pruefungen, %lu FEHL -- %s\n", faelle, fehl, fehl ? "FEHL" : "OK");
    return fehl ? 1 : 0;
}
