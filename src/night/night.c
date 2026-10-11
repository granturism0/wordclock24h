/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * night.c - night time routines
 *
 * Copyright (c) 2016-2026 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include "eep.h"
#include "eeprom-data.h"
#include "night.h"
#include "log.h"

/*--------------------------------------------------------------------------------------------------------------------------------------
 * globals:
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
NIGHT_TIME              night_time[MAX_NIGHT_TIMES];
NIGHT_TIME              ambilight_night_time[MAX_NIGHT_TIMES];

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read configuration data from EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
night_read_data_from_eep (uint32_t eep_version)
{
    unsigned char   night_time_buf[EEPROM_DATA_SIZE_NIGHT_TIME];
    uint_fast8_t    i;
    uint_fast8_t    offset;
    uint_fast8_t    reinit = 0;
    uint_fast8_t    rtc = 0;

    if (eep_is_up)
    {
        if (eep_version >= EEPROM_VERSION_1_7)
        {
            if (eep_read (EEPROM_DATA_OFFSET_NIGHT_TIME, night_time_buf, EEPROM_DATA_SIZE_NIGHT_TIME))
            {
                for (offset = 0, i = 0; i < MAX_NIGHT_TIMES; i++)
                {
                    night_time[i].flags      = night_time_buf[offset++];
                    night_time[i].minutes    = night_time_buf[offset++] * 60;
                    night_time[i].minutes   += night_time_buf[offset++];

                    if (night_time[i].minutes > 24 * 60)                                // invalid EEPROM values?
                    {
                        night_time[i].flags      = 0;
                        night_time[i].minutes    = 0;
                        reinit = 1;                                                     // must re-initialize eeprom data
                    }
                }

                rtc = 1;
            }

            if (eep_version >= EEPROM_VERSION_2_5)
            {
                if (eep_read (EEPROM_DATA_OFFSET_AMBI_NIGHT_TIME, night_time_buf, EEPROM_DATA_SIZE_AMBI_NIGHT_TIME))
                {
                    for (offset = 0, i = 0; i < MAX_NIGHT_TIMES; i++)
                    {
                        ambilight_night_time[i].flags      = night_time_buf[offset++];
                        ambilight_night_time[i].minutes    = night_time_buf[offset++] * 60;
                        ambilight_night_time[i].minutes   += night_time_buf[offset++];

                        if (ambilight_night_time[i].minutes > 24 * 60)                                // invalid EEPROM values?
                        {
                            ambilight_night_time[i].flags      = 0;
                            ambilight_night_time[i].minutes    = 0;
                            reinit = 1;                                                     // must re-initialize eeprom data
                        }
                    }
                }
            }

            if (reinit)
            {
                night_write_data_to_eep ();
            }
        }
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write configuration data to EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
night_write_data_to_eep (void)
{
    unsigned char   night_time_buf[EEPROM_DATA_SIZE_NIGHT_TIME];
    uint_fast8_t    i;
    uint_fast8_t    offset;
    uint_fast8_t    rtc = 0;

    if (eep_is_up)
    {
        for (offset = 0, i = 0; i < MAX_NIGHT_TIMES; i++)
        {
            night_time_buf[offset++] = night_time[i].flags;
            night_time_buf[offset++] = night_time[i].minutes / 60;                      // hh
            night_time_buf[offset++] = night_time[i].minutes % 60;                      // mm
        }

        if (eep_write (EEPROM_DATA_OFFSET_NIGHT_TIME, night_time_buf, EEPROM_DATA_SIZE_NIGHT_TIME))
        {
            for (offset = 0, i = 0; i < MAX_NIGHT_TIMES; i++)
            {
                night_time_buf[offset++] = ambilight_night_time[i].flags;
                night_time_buf[offset++] = ambilight_night_time[i].minutes / 60;        // hh
                night_time_buf[offset++] = ambilight_night_time[i].minutes % 60;        // mm
            }

            if (eep_write (EEPROM_DATA_OFFSET_AMBI_NIGHT_TIME, night_time_buf, EEPROM_DATA_SIZE_AMBI_NIGHT_TIME))
            {
                rtc = 1;
            }
        }
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * check weekday of a night time entry
 *
 * A69: unveraendert aus night_check_night_times() herausgeloest, damit night_last_event() DIESELBE
 * Pruefung nutzt. Tage 0 = So .. 6 = Sa; ein Feldwert 7 passt auf keinen Tag (wie bisher).
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
night_wday_valid (uint_fast8_t flags, uint_fast8_t wday)
{
    uint_fast8_t   from_wday    = (flags & NIGHT_TIME_FROM_DAY_MASK) >> 3;
    uint_fast8_t   to_wday      = (flags & NIGHT_TIME_TO_DAY_MASK);

    if (from_wday == to_wday)                                               // e.g. Su-Su 0-0
    {
        return (wday == from_wday);
    }
    else if (from_wday < to_wday)                                           // e.g. Mo-Fr 1-5
    {
        return (wday >= from_wday && wday <= to_wday);
    }
    return ! (wday > to_wday && wday < from_wday);                          // e.g. Sa-We 6-3
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * last night time event of the display list (A69, specs/paket-2026-10-11, design.md 3.1)
 *
 * Liefert das JUENGSTE vergangene Ereignis der Displayliste: aktiv, fuer seinen Wochentag gueltig,
 * hoechstens eine Woche zurueck (k = 0: heute bis einschliesslich Minute m; k = 1..7: die Vortage,
 * k = 7 ist derselbe Wochentag vor einer Woche). *backp = Abstand in Minuten (0 = diese Minute).
 * Nur die Displayliste (Ent-2 (1)); ein Aus schaltet beim Anwenden das Ambilight mit, wie der Timer.
 * minutes == 1440 laesst night_read_data_from_eep() zu; night_check_night_times() trifft es nie, also
 * bleibt es auch hier unwirksam -- sonst galte es als 00:00 des Folgetags.
 * Gleichstand (zwei Timer derselben Minute): der erste in Tabellenreihenfolge. Der Timer selbst nimmt
 * dagegen den ersten, der den Zustand AENDERT -- verschieden nur bei Aus und Ein in derselben Minute.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
night_last_event (uint_fast8_t wday, uint_fast16_t m, uint_fast16_t * backp)
{
    NIGHT_TIME *    ntp;
    uint_fast8_t    i;
    uint_fast8_t    k;
    uint_fast8_t    rtc = 0;
    uint_fast16_t   back;
    uint_fast16_t   best = 0xFFFF;

    for (k = 0; k < 8; k++)
    {
        for (i = 0, ntp = night_time; i < MAX_NIGHT_TIMES; i++, ntp++)
        {
            if ((ntp->flags & NIGHT_TIME_FLAG_ACTIVE) && ntp->minutes < 24 * 60 && (k || ntp->minutes <= m) &&
                night_wday_valid (ntp->flags, (wday + 7 - k) % 7))
            {
                back = k * (24 * 60) + m - ntp->minutes;

                if (back < best)
                {
                    best = back;
                    rtc  = (ntp->flags & NIGHT_TIME_FLAG_SWITCH_ON) ? NIGHT_EVENT_ON : NIGHT_EVENT_OFF;
                }
            }
        }
    }

    *backp = best;
    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * check night_times
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
night_check_night_times (uint_fast8_t is_ambilight, uint_fast8_t power_is_on, uint_fast8_t wday, uint_fast16_t m)
{
    NIGHT_TIME *    ntp;
    uint_fast8_t    i;
    uint_fast8_t    condition;
    uint_fast8_t    rtc = 0;

    if (is_ambilight)
    {
        ntp = ambilight_night_time;
    }
    else
    {
        ntp = night_time;
    }

    for (i = 0; i < MAX_NIGHT_TIMES; i++, ntp++)
    {
        if ((ntp->flags & NIGHT_TIME_FLAG_ACTIVE) && ntp->minutes == m)
        {
            condition = ntp->flags & NIGHT_TIME_FLAG_SWITCH_ON;

            if (power_is_on)
            {
                condition = !condition;
            }

            if (condition)
            {
                rtc = night_wday_valid (ntp->flags, wday);

                if (rtc)
                {
                    break;
                }
            }
        }
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * init night times
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
night_init (void)
{
    return;                                                                 // nothing to do
}
