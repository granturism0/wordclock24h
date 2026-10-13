/* Eingefroren aus release/3.2.28-3.2.31-1.4.95, src/night/night.c -- Referenz fuer A69 (Minutengenau). */
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
                uint_fast8_t   from_wday    = (ntp->flags & NIGHT_TIME_FROM_DAY_MASK) >> 3;
                uint_fast8_t   to_wday      = (ntp->flags & NIGHT_TIME_TO_DAY_MASK);

                if (from_wday == to_wday)                                       // e.g. Su-Su 0-0
                {
                    rtc = (wday == from_wday);
                }
                else if (from_wday < to_wday)                                   // e.g. Mo-Fr 1-5
                {
                    rtc = (wday >= from_wday && wday <= to_wday);
                }
                else                                                            // e.g. Sa-We 6-3
                {
                    rtc = ! (wday > to_wday && wday < from_wday);
                }

                if (rtc)
                {
                    break;
                }
            }
        }
    }

    return rtc;
}
