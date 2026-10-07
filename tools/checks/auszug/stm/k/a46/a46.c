/* Pruefstand A46 (S.19g, AKS.13 / AKH.4): drei Verwerfungspfade am gemeinsamen Zaehler.
 * Echt: esp8266_cmd_reject() (main.c), tftled_layout_get_line() (tftled.c), esp_diffs_read_icon()
 * (esp-spiffs.c), htoi() (base.c). Nachgebildet: Anzeige, ESP-Sendeweg, Log (zaehlt Zeilen).
 * Je Pfad zehn verworfene Eingaben: Der gemeinsame Zaehler muss um zehn steigen, und es duerfen
 * hoechstens die gedrosselten Meldungen erscheinen (die ersten vier, dann jede fuenfzigste). */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pruefstand.h"
#ifdef ZIELTYPEN
#define uint_fast8_t    uint32_t
#define uint_fast16_t   uint32_t
#define int_fast16_t    int32_t
#define int_fast8_t     int32_t
typedef char zp_fast_pruefung[(sizeof (uint_fast8_t) == 4) ? 1 : -1];
#endif
#include "esp8266.h"
ESP8266_GLOBALS esp8266;
#define WC_ROWS 10
#define WC_COLUMNS 11
#define DSP_DISPLAY_LEDS (WC_ROWS * WC_COLUMNS)
typedef struct { uint_fast8_t rows, cols; uint8_t colors[WC_ROWS * WC_COLUMNS]; uint8_t animation_on[WC_ROWS * WC_COLUMNS]; uint8_t animation_off[WC_ROWS * WC_COLUMNS]; } DISPLAY_ICON;
static char layout_table[DSP_DISPLAY_LEDS];
static void tftled_layout (uint_fast8_t r) { (void) r; }
static void tftled_redraw_display (void) {}
void esp8266_send_cmd (const char * c, const char * a, uint_fast8_t l) { (void) c; (void) a; (void) l; }
static int logs;
static char lastlog[200];
#include <stdarg.h>
static int log_printf (const char * f, ...) { va_list ap; va_start (ap, f); vsnprintf (lastlog, sizeof (lastlog), f, ap); va_end (ap); logs++; return 0; }
#ifdef ALT_STATIC
#endif
static uint32_t esp8266_cmd_reject_cnt;
#include SNIPPET

static int fails, cases;
#define CHECK(name, cond) do { cases++; if (cond) printf ("  OK    %s\n", name); else { fails++; printf ("  FEHL  %s\n", name); } } while (0)

static void
pruefe (const char * pfad, uint32_t cnt0, int logs0, int wie_viele)
{
    char n[120];
    snprintf (n, sizeof (n), "%-28s Zaehler +%d (ist +%u)", pfad, wie_viele, (unsigned) (esp8266_cmd_reject_cnt - cnt0));
    CHECK (n, esp8266_cmd_reject_cnt - cnt0 == (uint32_t) wie_viele);
    snprintf (n, sizeof (n), "%-28s gedrosselt gemeldet (%d Zeilen)", pfad, logs - logs0);
    CHECK (n, logs - logs0 <= 4);
}

int
main (void)
{
    DISPLAY_ICON    di;
    int             i, l0;
    uint32_t        c0;

    printf ("[A46] drei Verwerfungspfade am gemeinsamen Zaehler\n");
    esp8266_cmd_reject_cnt = 0; logs = 0;

    c0 = esp8266_cmd_reject_cnt; l0 = logs;
    for (i = 0; i < 10; i++) { char s[2] = { (char) (i ? '7' : 0), 0 }; tftled_layout_get_line (s); }
    pruefe ("DISP-Zeile zu kurz", c0, l0, 10);

    c0 = esp8266_cmd_reject_cnt; l0 = logs;
    for (i = 0; i < 10; i++) { icon_block = 0; strcpy (esp8266.u.filedata, "0a0b0001"); esp_diffs_read_icon (&di); }
    pruefe ("ICON-Kopf zu kurz", c0, l0, 10);

    c0 = esp8266_cmd_reject_cnt; l0 = logs;
    for (i = 0; i < 10; i++)
    {
        icon_block = 0; strcpy (esp8266.u.filedata, "0202000400000000"); esp_diffs_read_icon (&di);   /* gueltiger Kopf */
        strcpy (esp8266.u.filedata, "0102030"); esp_diffs_read_icon (&di);                             /* sieben Zeichen: ungerader Rest */
    }
    pruefe ("ICON-Daten, ungerades Byte", c0, l0, 10);

    c0 = esp8266_cmd_reject_cnt; l0 = logs;
    icon_block = 0; strcpy (esp8266.u.filedata, "0202000400000000"); esp_diffs_read_icon (&di);
    strcpy (esp8266.u.filedata, "01020304"); esp_diffs_read_icon (&di);
    CHECK ("ICON-Daten, gerade Laenge: nichts verworfen", esp8266_cmd_reject_cnt == c0 && di.colors[3] == 4);
    printf ("  letzte Meldung: %s", lastlog);

    printf ("\n%d Faelle, %d fehlgeschlagen\n", cases, fails);
    return fails ? 1 : 0;
}
