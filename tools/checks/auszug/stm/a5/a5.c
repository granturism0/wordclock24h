/* Pruefstand A5 (AKS.15 STM-Teil, AKS.17): rtc_get_temperature_index() aus src/rtc/rtc.c, eingebunden
 * wie gelesen; nachgebildet ist nur rtc_read() mit festen Registerbytes 0x11/0x12.
 * Zieltypen: -DZIELTYPEN macht uint_fast8_t/uint_fast16_t/int_fast*_t 32 Bit breit wie arm-none-eabi.
 */
#include <stdio.h>
#include <stdint.h>
#include "pruefstand.h"
#ifdef ZIELTYPEN
#define uint_fast8_t    uint32_t
#define uint_fast16_t   uint32_t
#define int_fast8_t     int32_t
#define int_fast16_t    int32_t
typedef char zp_fast_pruefung[(sizeof (uint_fast8_t) == 4 && sizeof (int_fast16_t) == 4) ? 1 : -1];
#endif

#define DS3231_TEMP_REG_HI      0x11

typedef struct
{
    uint_fast8_t    rtc_is_up;
    int_fast8_t     rtc_temp_correction;
    uint_fast8_t    rtc_temperature_index;
#ifdef HAVE_HALF
    uint16_t        rtc_temp_half_deg;
#endif
} RTC_GLOBALS;
#define RTC_TEMP_HALF_DEG_NONE  0x8000

static RTC_GLOBALS  grtc;
static uint8_t      reg[2];
static int          read_ok;

static uint_fast8_t
rtc_read (uint_fast8_t start_addr, uint8_t * buffer, uint_fast8_t cnt)
{
    if (start_addr != DS3231_TEMP_REG_HI || cnt != 2 || ! read_ok) return 0;
    buffer[0] = reg[0]; buffer[1] = reg[1];
    return 1;
}

#include SNIPPET

static int fails, cases;

static void
fall (const char * name, uint8_t hi, uint8_t lo, int corr, int ok, int want21, int want49)
{
    uint_fast8_t    idx;
    int             half = 0;

    grtc.rtc_is_up = 1; grtc.rtc_temp_correction = (int_fast8_t) corr;
    reg[0] = hi; reg[1] = lo; read_ok = ok;
    idx = rtc_get_temperature_index ();
#ifdef HAVE_HALF
    half = grtc.rtc_temp_half_deg;
#else
    half = want49;                                  // alter Stand: Index 49 gibt es nicht, geprueft wird nur Index 21
#endif
    cases++;
    if ((int) idx == want21 && half == want49 && (int) grtc.rtc_temperature_index == want21)
        printf ("  OK    %-38s %02X %02X k=%+3d  -> 21=%3d 49=0x%04x\n", name, hi, lo, corr, (int) idx, half & 0xFFFF);
    else
    {
        fails++;
        printf ("  FEHL  %-38s %02X %02X k=%+3d  -> 21=%3d 49=0x%04x  (erwartet 21=%d 49=0x%04x)\n",
                name, hi, lo, corr, (int) idx, half & 0xFFFF, want21, want49 & 0xFFFF);
    }
}

#define H(x) ((int) (uint16_t) (int16_t) (x))       /* halbe Grad als int16 im Zweierkomplement */

int
main (void)
{
    printf ("[A5] Registerbytes -> Index 21 (0..250, begrenzt) und Index 49 (int16, halbe Grad)\n");
    fall ("-10,0 Grad",                     0xF6, 0x00,   0, 1,   0, H(-20));
    fall ("-0,5 Grad",                      0xFF, 0x80,   0, 1,   0, H(-1));
    fall ("-1,0 Grad",                      0xFF, 0x00,   0, 1,   0, H(-2));
    fall ("-0,25 Grad (Bit 6 und 7)",       0xFF, 0xC0,   0, 1,   0, H(-1));
    fall ("0,0 Grad",                       0x00, 0x00,   0, 1,   0, H(0));
    fall ("+24,5 Grad",                     0x18, 0x80,   0, 1,  49, H(49));
    fall ("+24,0 Grad, Bit 6 allein",       0x18, 0x40,   0, 1,  48, H(48));
    fall ("+24,0 Grad, Bit 7 (=> +24,5)",   0x18, 0x80,   0, 1,  49, H(49));
    fall ("+24,0 Grad, Korrektur +20",      0x18, 0x00,  20, 1,  28, H(28));
    fall ("+24,0 Grad, Korrektur -20",      0x18, 0x00, -20, 1,  68, H(68));
    fall ("-1,0 Grad, Korrektur -20",       0xFF, 0x00, -20, 1,  18, H(18));
    fall ("+5,0 Grad, Korrektur +20",       0x05, 0x00,  20, 1,   0, H(-10));
    fall ("+127,75 Grad (Begrenzung 250)",  0x7F, 0xC0,   0, 1, 250, H(255));
    fall ("-128 Grad, Korrektur +20",       0x80, 0x00,  20, 1,   0, H(-276));
    fall ("Lesefehler (I2C)",               0x18, 0x80,   0, 0,   0, H(RTC_TEMP_HALF_DEG_NONE));
    grtc.rtc_is_up = 0; read_ok = 1;
    {
        uint_fast8_t idx = rtc_get_temperature_index ();
        int half =
#ifdef HAVE_HALF
            grtc.rtc_temp_half_deg;
#else
            0x8000;                                 // alter Stand: nur Index 21 pruefbar
#endif
        cases++;
        if (idx == 0xFF && half == 0x8000) printf ("  OK    ohne RTC                               -> 21=255 49=0x8000\n");
        else { fails++; printf ("  FEHL  ohne RTC -> 21=%d 49=0x%04x\n", (int) idx, half & 0xFFFF); }
    }
    printf ("\n%d Faelle, %d fehlgeschlagen\n", cases, fails);
    return fails ? 1 : 0;
}
