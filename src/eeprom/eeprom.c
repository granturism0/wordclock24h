/*-------------------------------------------------------------------------------------------------------------------------------------------
 * eeprom.c - EEPROM routines
 *
 * Copyright (c) 2014-2026 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */

#if ! defined(BLACK_BOARD)                                      // flash only on STM32F407 Black Board, all other: EEPROM

#include "eeprom.h"
#include "i2c.h"
#include "log.h"                                                // Messzeile in eeprom_write()

#undef  UART_PREFIX                                             // log.h hat UART_PREFIX auf "log" gesetzt
#define UART_PREFIX             esp8266                         // esp8266_uart_rxdrops() fuer d= der Messzeile
#include "uart.h"                                               // absichtlich ohne Include-Guard, je Praefix neu einbindbar

#define SHOW_SIZES              0

#if SHOW_SIZES == 1
#include "eeprom-data.h"
#include "log.h"
#endif

#define EEPROM_FIRST_ADDR       0xA0                            // I2C address << 1
#define EEPROM_WAITSTATES       15                              // we have to wait 15ms after each write cycle

/*--------------------------------------------------------------------------------------------------------------------------------------
 * Seitengroesse fuer eeprom_write() (Paket 2026-10-09, C3, Ent-7), Build-Konstante je Ziel:
 *   F411 (Platine V2): 32 Byte -- der AT24C32 der Platine hat 32-Byte-Seiten.
 *   F103:               8 Byte -- der Baustein des F103-Aufbaus ist nicht belegt; kleiner ist immer
 *                                 sicher, nur langsamer. Festgelegt vom Nutzer.
 * NIE groesser als die echte Seite: Ein Schreiben ueber das Seitenende bricht im Baustein auf den
 * Anfang DERSELBEN Seite um und ueberschreibt dort fremde Einstellungen, ohne dass i2c_write() einen
 * Fehler meldet. Muss 4096 teilen (Zweierpotenz), damit die Seitenrechnung auch ueber das
 * Speicherende hinaus zur Adressumwicklung des Bausteins passt.
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
#if defined (STM32F4XX)
#define EEPROM_PAGE_SIZE        32
#elif defined (STM32F10X)
#define EEPROM_PAGE_SIZE        8
#else
#error EEPROM_PAGE_SIZE: Ziel ohne Seitengroesse
#endif

uint_fast8_t                    eeprom_is_up = 0;
volatile uint_fast8_t           eeprom_ms_tick;                 // should be set every 1 ms by IRQ, see main.c

static  uint_fast8_t            eeprom_addr;


/*--------------------------------------------------------------------------------------------------------------------------------------
 * eeprom_waitstates() - wait 15 ms after each write cycle
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static void
eeprom_waitstates (void)
{
    uint_fast16_t  cnt = 0;

    while (1)
    {
        if (eeprom_ms_tick)
        {
            eeprom_ms_tick = 0;

            cnt++;

            if (cnt > EEPROM_WAITSTATES)
            {
                break;
            }
        }
    }
}


/*-------------------------------------------------------------------------------------------------------------------------------------------
 * initialize EEPROM functions
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
eeprom_init (uint32_t clockspeed)
{
    uint_fast8_t    i;
    uint8_t         value;

    i2c_init (clockspeed);

    for (i = 0; i < 8; i++)                                     // test read 0xA0, 0xA2, 0xA4, 0xA6 .. 0xAE
    {
        eeprom_addr = EEPROM_FIRST_ADDR | (i << 1);

        if (i2c_read (eeprom_addr, 0x00, 1, &value, 1) == I2C_OK)
        {
            eeprom_is_up = 1;
            break;
        }
    }

#if SHOW_SIZES == 1
    log_printf ("EEPROM_DATA_OFFSET_VERSION                  = %4d %4d\r\n", EEPROM_DATA_OFFSET_VERSION, EEPROM_DATA_SIZE_VERSION);
    log_printf ("EEPROM_DATA_OFFSET_IRMP_DATA                = %4d %4d\r\n", EEPROM_DATA_OFFSET_IRMP_DATA, EEPROM_DATA_SIZE_IRMP_DATA);
    log_printf ("EEPROM_DATA_OFFSET_DSP_COLORS               = %4d %4d\r\n", EEPROM_DATA_OFFSET_DSP_COLORS, EEPROM_DATA_SIZE_DSP_COLORS);
    log_printf ("EEPROM_DATA_OFFSET_DISPLAY_MODE             = %4d %4d\r\n", EEPROM_DATA_OFFSET_DISPLAY_MODE, EEPROM_DATA_SIZE_DISPLAY_MODE);
    log_printf ("EEPROM_DATA_OFFSET_ANIMATION_MODE           = %4d %4d\r\n", EEPROM_DATA_OFFSET_ANIMATION_MODE, EEPROM_DATA_SIZE_ANIMATION_MODE);
    log_printf ("EEPROM_DATA_OFFSET_COLOR_ANIMATION_MODE     = %4d %4d\r\n", EEPROM_DATA_OFFSET_COLOR_ANIMATION_MODE, EEPROM_DATA_SIZE_COLOR_ANIMATION_MODE);
    log_printf ("EEPROM_DATA_OFFSET_TIMESERVER               = %4d %4d\r\n", EEPROM_DATA_OFFSET_TIMESERVER, EEPROM_DATA_SIZE_TIMESERVER);
    log_printf ("EEPROM_DATA_OFFSET_TIMEZONE                 = %4d %4d\r\n", EEPROM_DATA_OFFSET_TIMEZONE, EEPROM_DATA_SIZE_TIMEZONE);
    log_printf ("EEPROM_DATA_OFFSET_DISPLAY_FLAGS            = %4d %4d\r\n", EEPROM_DATA_OFFSET_DISPLAY_FLAGS, EEPROM_DATA_SIZE_DISPLAY_FLAGS);
    log_printf ("EEPROM_DATA_OFFSET_BRIGHTNESS               = %4d %4d\r\n", EEPROM_DATA_OFFSET_BRIGHTNESS, EEPROM_DATA_SIZE_BRIGHTNESS);
    log_printf ("EEPROM_DATA_OFFSET_AUTO_BRIGHTNESS          = %4d %4d\r\n", EEPROM_DATA_OFFSET_AUTO_BRIGHTNESS, EEPROM_DATA_SIZE_AUTO_BRIGHTNESS);
    log_printf ("EEPROM_DATA_OFFSET_NIGHT_TIME               = %4d %4d\r\n", EEPROM_DATA_OFFSET_NIGHT_TIME, EEPROM_DATA_SIZE_NIGHT_TIME);
    log_printf ("EEPROM_DATA_OFFSET_AMBI_COLORS              = %4d %4d\r\n", EEPROM_DATA_OFFSET_AMBI_COLORS, EEPROM_DATA_SIZE_AMBI_COLORS);
    log_printf ("EEPROM_DATA_OFFSET_AMBI_BRIGHTNESS          = %4d %4d\r\n", EEPROM_DATA_OFFSET_AMBI_BRIGHTNESS, EEPROM_DATA_SIZE_AMBI_BRIGHTNESS);
    log_printf ("EEPROM_DATA_OFFSET_AMBI_MODE                = %4d %4d\r\n", EEPROM_DATA_OFFSET_AMBI_MODE, EEPROM_DATA_SIZE_AMBI_MODE);
    log_printf ("EEPROM_DATA_OFFSET_AMBI_LEDS                = %4d %4d\r\n", EEPROM_DATA_OFFSET_AMBI_LEDS, EEPROM_DATA_SIZE_AMBI_LEDS);
    log_printf ("EEPROM_DATA_OFFSET_AMBI_OFFSET_SEC0         = %4d %4d\r\n", EEPROM_DATA_OFFSET_AMBI_OFFSET_SEC0, EEPROM_DATA_SIZE_AMBI_OFFSET_SEC0);
    log_printf ("EEPROM_DATA_OFFSET_RTC_TEMP_CORR            = %4d %4d\r\n", EEPROM_DATA_OFFSET_RTC_TEMP_CORR, EEPROM_DATA_SIZE_RTC_TEMP_CORR);
    log_printf ("EEPROM_DATA_OFFSET_DS18XX_TEMP_CORR         = %4d %4d\r\n", EEPROM_DATA_OFFSET_DS18XX_TEMP_CORR, EEPROM_DATA_SIZE_DS18XX_TEMP_CORR);
    log_printf ("EEPROM_DATA_OFFSET_NOT_USED_01              = %4d %4d\r\n", EEPROM_DATA_OFFSET_NOT_USED_01, EEPROM_DATA_SIZE_NOT_USED_01);
    log_printf ("EEPROM_DATA_OFFSET_LDR_MIN_VALUE            = %4d %4d\r\n", EEPROM_DATA_OFFSET_LDR_MIN_VALUE, EEPROM_DATA_SIZE_LDR_MIN_VALUE);
    log_printf ("EEPROM_DATA_OFFSET_LDR_MAX_VALUE            = %4d %4d\r\n", EEPROM_DATA_OFFSET_LDR_MAX_VALUE, EEPROM_DATA_SIZE_LDR_MAX_VALUE);
    log_printf ("EEPROM_DATA_OFFSET_ANIMATION_VALUES         = %4d %4d\r\n", EEPROM_DATA_OFFSET_ANIMATION_VALUES, EEPROM_DATA_SIZE_ANIMATION_VALUES);
    log_printf ("EEPROM_DATA_OFFSET_COLOR_ANIMATION_VALUES   = %4d %4d\r\n", EEPROM_DATA_OFFSET_COLOR_ANIMATION_VALUES, EEPROM_DATA_SIZE_COLOR_ANIMATION_VALUES);
    log_printf ("EEPROM_DATA_OFFSET_AMBILIGHT_MODE_VALUES    = %4d %4d\r\n", EEPROM_DATA_OFFSET_AMBILIGHT_MODE_VALUES, EEPROM_DATA_SIZE_AMBILIGHT_MODE_VALUES);
    log_printf ("EEPROM_DATA_OFFSET_DSP_W_COLOR              = %4d %4d\r\n", EEPROM_DATA_OFFSET_DSP_W_COLOR, EEPROM_DATA_SIZE_DSP_W_COLOR);
    log_printf ("EEPROM_DATA_OFFSET_AMBI_W_COLOR             = %4d %4d\r\n", EEPROM_DATA_OFFSET_AMBI_W_COLOR, EEPROM_DATA_SIZE_AMBI_W_COLOR);
    log_printf ("EEPROM_DATA_OFFSET_WEATHER_APPID            = %4d %4d\r\n", EEPROM_DATA_OFFSET_WEATHER_APPID, EEPROM_DATA_SIZE_WEATHER_APPID);
    log_printf ("EEPROM_DATA_OFFSET_WEATHER_CITY             = %4d %4d\r\n", EEPROM_DATA_OFFSET_WEATHER_CITY, EEPROM_DATA_SIZE_WEATHER_CITY);
    log_printf ("EEPROM_DATA_OFFSET_WEATHER_LON              = %4d %4d\r\n", EEPROM_DATA_OFFSET_WEATHER_LON, EEPROM_DATA_SIZE_WEATHER_LON);
    log_printf ("EEPROM_DATA_OFFSET_WEATHER_LAT              = %4d %4d\r\n", EEPROM_DATA_OFFSET_WEATHER_LAT, EEPROM_DATA_SIZE_WEATHER_LAT);
    log_printf ("EEPROM_DATA_OFFSET_OVERLAY_INTERVALS        = %4d %4d\r\n", EEPROM_DATA_OFFSET_OVERLAY_INTERVALS, EEPROM_DATA_SIZE_OVERLAY_INTERVALS);
    log_printf ("EEPROM_DATA_OFFSET_AMBI_NIGHT_TIME          = %4d %4d\r\n", EEPROM_DATA_OFFSET_AMBI_NIGHT_TIME, EEPROM_DATA_SIZE_AMBI_NIGHT_TIME);
    log_printf ("EEPROM_DATA_OFFSET_DIMMED_DISPLAY_COLORS    = %4d %4d\r\n", EEPROM_DATA_OFFSET_DIMMED_DISPLAY_COLORS, EEPROM_DATA_SIZE_DIMMED_DISPLAY_COLORS);
    log_printf ("EEPROM_DATA_OFFSET_UPDATE_HOSTNAME          = %4d %4d\r\n", EEPROM_DATA_OFFSET_UPDATE_HOSTNAME, EEPROM_DATA_SIZE_UPDATE_HOSTNAME);
    log_printf ("EEPROM_DATA_OFFSET_UPDATE_PATH              = %4d %4d\r\n", EEPROM_DATA_OFFSET_UPDATE_PATH, EEPROM_DATA_SIZE_UPDATE_PATH);
    log_printf ("EEPROM_DATA_OFFSET_TICKER_DECELERATION      = %4d %4d\r\n", EEPROM_DATA_OFFSET_TICKER_DECELERATION, EEPROM_DATA_SIZE_TICKER_DECELERATION);
    log_printf ("EEPROM_DATA_OFFSET_DFPLAYER_VOLUME          = %4d %4d\r\n", EEPROM_DATA_OFFSET_DFPLAYER_VOLUME, EEPROM_DATA_SIZE_DFPLAYER_VOLUME);
    log_printf ("EEPROM_DATA_OFFSET_DFPLAYER_SILENCE_START   = %4d %4d\r\n", EEPROM_DATA_OFFSET_DFPLAYER_SILENCE_START, EEPROM_DATA_SIZE_DFPLAYER_SILENCE_START);
    log_printf ("EEPROM_DATA_OFFSET_DFPLAYER_SILENCE_STOP    = %4d %4d\r\n", EEPROM_DATA_OFFSET_DFPLAYER_SILENCE_STOP, EEPROM_DATA_SIZE_DFPLAYER_SILENCE_STOP);
    log_printf ("EEPROM_DATA_OFFSET_DFPLAYER_MODE            = %4d %4d\r\n", EEPROM_DATA_OFFSET_DFPLAYER_MODE, EEPROM_DATA_SIZE_DFPLAYER_MODE);
    log_printf ("EEPROM_DATA_OFFSET_DFPLAYER_BELL_FLAGS      = %4d %4d\r\n", EEPROM_DATA_OFFSET_DFPLAYER_BELL_FLAGS, EEPROM_DATA_SIZE_DFPLAYER_BELL_FLAGS);
    log_printf ("EEPROM_DATA_OFFSET_DFPLAYER_SPEAK_CYCLE     = %4d %4d\r\n", EEPROM_DATA_OFFSET_DFPLAYER_SPEAK_CYCLE, EEPROM_DATA_SIZE_DFPLAYER_SPEAK_CYCLE);
    log_printf ("EEPROM_DATA_OFFSET_ALARM_TIME               = %4d %4d\r\n", EEPROM_DATA_OFFSET_ALARM_TIME, EEPROM_DATA_SIZE_ALARM_TIME);
    log_printf ("EEPROM_DATA_OFFSET_N_OVERLAYS               = %4d %4d\r\n", EEPROM_DATA_OFFSET_N_OVERLAYS, EEPROM_DATA_SIZE_N_OVERLAYS);
    log_printf ("EEPROM_DATA_OFFSET_OVERLAY                  = %4d %4d\r\n", EEPROM_DATA_OFFSET_OVERLAY, EEPROM_DATA_SIZE_OVERLAY);
    log_printf ("EEPROM_DATA_OFFSET_AMBI_MARKER_COLORS       = %4d %4d\r\n", EEPROM_DATA_OFFSET_AMBI_MARKER_COLORS, EEPROM_DATA_SIZE_AMBI_MARKER_COLORS);
    log_printf ("EEPROM_DATA_OFFSET_AMBI_MARKER_W_COLOR      = %4d %4d\r\n", EEPROM_DATA_OFFSET_AMBI_MARKER_W_COLOR, EEPROM_DATA_SIZE_AMBI_MARKER_W_COLOR);
    log_printf ("EEPROM_DATA_OFFSET_DATE_TICKER_FORMAT       = %4d %4d\r\n", EEPROM_DATA_OFFSET_DATE_TICKER_FORMAT, EEPROM_DATA_SIZE_DATE_TICKER_FORMAT);
    log_printf ("EEPROM_DATA_OFFSET_DIMMED_AMBILIGHT_COLORS  = %4d %4d\r\n", EEPROM_DATA_OFFSET_DIMMED_AMBILIGHT_COLORS, EEPROM_DATA_SIZE_DIMMED_AMBILIGHT_COLORS);
#endif

    return eeprom_is_up;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * get address of EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
eeprom_get_address (void)
{
    return eeprom_addr;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
eeprom_read (uint_fast16_t start_addr, uint8_t * buffer, uint_fast16_t cnt)
{
    uint_fast8_t    rtc;

    if (eeprom_is_up)
    {
        if (i2c_read (eeprom_addr, start_addr, 1, buffer, cnt) == I2C_OK)
        {
            rtc = 1;
        }
        else
        {
            rtc = 0;
        }
    }
    else
    {
        rtc = 0;
    }
    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
/*--------------------------------------------------------------------------------------------------------------------------------------
 * Seitenweise (Paket 2026-10-09, C3): Der Bereich wird an den Seitengrenzen (EEPROM_PAGE_SIZE)
 * zerlegt. Je Seite EIN Lesen zum Vergleich, hoechstens EIN Schreiben -- vom ersten bis zum letzten
 * abweichenden Byte, also nie ueber das Seitenende -- und hoechstens EIN Wartezyklus. Eine Seite,
 * die schon so im EEPROM steht, kostet keinen Schreibzyklus.
 *
 * Der teure Teil ist nicht der I2C-Verkehr, sondern eeprom_waitstates(): EEPROM_WAITSTATES = 15,
 * nach G2 (BEFUNDE.md, L358) rund 24 ms je Zyklus samt I2C-Verkehr. Bis C3 fiel das je
 * geschriebenem BYTE an, seither je geschriebener SEITE. Waehrend des Wartens steht der Hauptloop
 * und niemand den
 * Empfangsring der ESP-Bruecke leert. Der Ring fasste zur Zeit dieser Messung 256 Byte und war
 * damit nach 22,2 ms voll; was danach kommt, verwirft die ISR still (BEFUNDE.md, L144: 702 Zeichen auf
 * einmal, dazu ein Einbruch der Hauptloop-Durchlaeufe auf 58 % ueber rund 4,25 s).
 *
 * Ein Lesezugriff kostet rund 0,1 ms und hat keine Wartezeit. Der Normalfall der PWA ist "alles
 * speichern, nichts hat sich geaendert" -- dort wurde mit dem Vergleich je Byte (vor C3, damalige
 * Messung) aus 240 ms fuer die Dimmkurve rund 2 ms und aus 960 ms fuer den Hostnamen rund 7 ms.
 *
 * Der Fehlerpfad bleibt unveraendert streng, und die Richtung ist mit Absicht gewaehlt:
 * Scheitert das LESEN, wird die ganze Seite geschrieben. Ein nicht lesbares Byte gilt nicht als
 * gleich -- sonst meldete ein I2C-Fehler "gespeichert", ohne dass je etwas im EEPROM gelandet
 * waere. Nur ein fehlgeschlagener SCHREIBzugriff bricht ab und liefert 0, wie bisher; danach
 * folgt keine weitere Seite. Im EEPROM steht dann ein Seiten-Praefix statt eines Byte-Praefixes.
 *
 * Nebeneffekt, nicht Zweck: Das EEPROM hat eine endliche Zahl Schreibzyklen je Zelle.
 *
 * Messzeile (Paket 2026-10-09, M.1, AKM.1): Nach jedem Aufruf mit mindestens einem Schreibzyklus
 * genau eine Zeile "eep a=<start> n=<anzahl> z=<zyklen> ms=<dauer> d=<vorher>/<nachher>", ohne
 * weitere Schwelle -- es gibt keine periodischen Schreiber (V.1 (2)). Kein Inhaltsbyte: Die
 * Bereiche tragen Zugangsdaten, und log_printf() geht auch als LOG-Zeile zum ESP.
 *   z   Aufrufe von eeprom_waitstates(), also Schreibzyklen
 *   ms  ueber diag_ticks() aus main.c, UNABHAENGIG von eeprom_ms_tick (Begruendung dort)
 *   d   verworfene Zeichen der ESP-Bruecke vor und nach dem Schreiben. Die Abhaengigkeit von der
 *       ESP-UART ist gewollt: Genau dieser Ring laeuft waehrend des Busy-Waits ueber (L144).
 * z zaehlt seit C3 Seiten mit Schreibzyklus, weiterhin je Aufruf von eeprom_waitstates().
 * Vor log_init() kann hier kein Zyklus laufen: eeprom_is_up wird erst in eeprom_init() gesetzt,
 * und main() ruft eep_init() nach log_init() und nach timer2_init().
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
eeprom_write (uint_fast16_t start_addr, uint8_t * buffer, uint_fast16_t cnt)
{
    uint_fast8_t rtc = 1;                                   // cnt == 0 ist kein Fehler; vorher war rtc hier uninitialisiert
    uint_fast16_t   m_a  = start_addr;                      // Messzeile: Werte vor der Schleife festhalten
    uint_fast16_t   m_n  = cnt;
    uint_fast16_t   m_z  = 0;
    uint_fast16_t   m_d  = esp8266_uart_rxdrops ();
    uint32_t        m_t  = diag_ticks ();
    uint8_t         page[EEPROM_PAGE_SIZE];                 // Vergleichspuffer, eine Seite
    uint_fast16_t   len;                                    // Byte bis zum Seitenende, hoechstens cnt
    uint_fast16_t   first;                                  // erstes abweichendes Byte der Seite
    uint_fast16_t   end;                                    // hinter dem letzten abweichenden Byte

    if (eeprom_is_up)
    {
        while (cnt)
        {
            len = EEPROM_PAGE_SIZE - (start_addr % EEPROM_PAGE_SIZE);   // auf einer Seitengrenze: die ganze Seite

            if (len > cnt)
            {
                len = cnt;
            }

            first = 0;                                      // Lesen gescheitert: ganze Teilseite gilt als verschieden
            end   = len;

            if (i2c_read (eeprom_addr, start_addr, 1, page, len) == I2C_OK)
            {
                while (first < len && page[first] == buffer[first])
                {
                    first++;
                }

                while (end > first && page[end - 1] == buffer[end - 1])
                {
                    end--;
                }
            }

            if (first < end)                                // sonst steht die Seite schon so da: kein Zyklus
            {
                if (i2c_write (eeprom_addr, start_addr + first, 1, buffer + first, end - first) != I2C_OK)
                {
                    rtc = 0;                                // keine weitere Seite
                    break;
                }
                eeprom_waitstates ();
                m_z++;
            }

            start_addr  += len;
            buffer      += len;
            cnt         -= len;
        }
    }
    else
    {
        rtc = 0;
    }

    if (m_z)
    {
        log_printf ("eep a=%u n=%u z=%u ms=%lu d=%u/%u\r\n", (unsigned int) m_a, (unsigned int) m_n, (unsigned int) m_z,
                    (unsigned long) ((diag_ticks () - m_t) / diag_ticks_per_ms), (unsigned int) m_d, (unsigned int) esp8266_uart_rxdrops ());
    }
    return rtc;
}

#endif // BLACK_BOARD
