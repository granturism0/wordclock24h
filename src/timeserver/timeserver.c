/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * timeserver.c - timeserver routines
 *
 * Copyright (c) 2014-2026 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include "base.h"
#include "timeserver.h"
#include "esp8266.h"
#include "eep.h"
#include "eeprom-data.h"
#include "log.h"

/*--------------------------------------------------------------------------------------------------------------------------------------
 * NTP server
 *
 * Default time server is ntp3.ptb.de (192.53.103.103)
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
#define NET_TIME_HOST           "192.53.103.103"                // IP address of default timeserver

/*--------------------------------------------------------------------------------------------------------------------------------------
 * Default GMT offset in seconds. For MEZ or MESZ, set it to 3600.
 * Summer time will be calculated automatically
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
#define NET_TIME_GMT_OFFSET     (GMT+1)                         // GMT offset, e.g. (GMT+1) for MEZ or MESZ

/*--------------------------------------------------------------------------------------------------------------------------------------
 * globals:
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
TIMESERVER_GLOBALS   timeserver =
{
    NET_TIME_HOST,                      // timeserver[MAX_IPADDR_LEN + 1]
    NET_TIME_GMT_OFFSET,                // timezone from -12 to +12
    1                                   // observe summertime
};

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read configuration data from EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
timeserver_read_data_from_eep (void)
{
    uint_fast8_t    rtc = 0;
    uint8_t         timeserver_buf[EEPROM_DATA_SIZE_TIMESERVER];
    uint8_t         tz[EEPROM_DATA_SIZE_TIMEZONE];

    memset (timeserver_buf, 0, sizeof (timeserver_buf));

    if (eep_is_up &&
        eep_read (EEPROM_DATA_OFFSET_TIMESERVER, timeserver_buf, EEPROM_DATA_SIZE_TIMESERVER) &&
        eep_read (EEPROM_DATA_OFFSET_TIMEZONE, tz, EEPROM_DATA_SIZE_TIMEZONE))
    {
        memcpy (timeserver.timeserver, timeserver_buf, EEPROM_DATA_SIZE_TIMESERVER);
        timeserver.timeserver[EEPROM_DATA_SIZE_TIMESERVER] = '\0';

        timeserver.timezone = tz[1];

        if (tz[0] == '-')       // old format: '-' or '+'
        {
            timeserver.timezone = -timeserver.timezone;
        }
        else if (tz[0] == '+')  // old format: '+'
        {
            if (timeserver.timezone == 1)   // MESZ
            {
                timeserver.observe_summertime = 1;
            }
        }
        else                    // new flags as bits: 0x01 = negative, 0x02 = observe summertime;
        {
             if (tz[0] & 0x01)
             {
                 timeserver.timezone = -timeserver.timezone;
             }

             if (tz[0] & 0x02)
             {
                 timeserver.observe_summertime = 1;
             }
             else
             {
                 timeserver.observe_summertime = 0;
             }
        }

        if (tz[1] > 12)                                                     // A62: dieselbe Grenze wie timeserver_set_timezone()
        {
            log_message ("time: zeitzone ungueltig, vorgabe");
            timeserver.timezone = NET_TIME_GMT_OFFSET;
        }

        rtc = 1;
    }
    else
    {
        strncpy ((char *) timeserver.timeserver, NET_TIME_HOST, EEPROM_DATA_SIZE_TIMESERVER - 1);
        timeserver.timeserver[EEPROM_DATA_SIZE_TIMESERVER - 1] = '\0';
        timeserver.timezone = NET_TIME_GMT_OFFSET;
        timeserver.observe_summertime = 1;
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save timeserver
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
timeserver_save_timeserver (void)
{
    uint_fast8_t    rtc = 0;
    uint8_t         timeserver_buf[EEPROM_DATA_SIZE_TIMESERVER];

    memset (timeserver_buf, 0, sizeof (timeserver_buf));
    strncpy ((char *) timeserver_buf, (char *) timeserver.timeserver, EEPROM_DATA_SIZE_TIMESERVER - 1);

    if (eep_is_up &&
        eep_write (EEPROM_DATA_OFFSET_TIMESERVER, timeserver_buf, EEPROM_DATA_SIZE_TIMESERVER))
    {
        rtc = 1;
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save timezone
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
timeserver_save_timezone (void)
{
    uint8_t         tz[EEPROM_MAX_TIMEZONE_LEN];
    uint_fast8_t    rtc = 0;

    if (timeserver.timezone >= 0)
    {
        tz[0] = 0x00;
        tz[1] = timeserver.timezone;
    }
    else
    {
        tz[0] = 0x01;
        tz[1] = -timeserver.timezone;
    }

    if (timeserver.observe_summertime)
    {
        tz[0] |= 0x02;
    }

    if (eep_is_up &&
        eep_write (EEPROM_DATA_OFFSET_TIMEZONE, tz, EEPROM_DATA_SIZE_TIMEZONE))
    {
        rtc = 1;
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write configuration data to EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
timeserver_write_data_to_eep (void)
{
    uint_fast8_t    rtc = 0;

    if (timeserver_save_timeserver () &&
        timeserver_save_timezone ())
    {
        rtc = 1;
    }

    return rtc;
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * set new timezone
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
timeserver_set_timezone (int_fast16_t newtimezone, uint_fast8_t observe_summertime)
{
    uint_fast8_t rtc = 0;

    log_printf ("New timezone: %d\r\n", newtimezone);

    if (newtimezone >= -12 && newtimezone <= 12)
    {
        if (timeserver.timezone != newtimezone || timeserver.observe_summertime != observe_summertime)
        {
            timeserver.timezone = newtimezone;
            timeserver.observe_summertime = observe_summertime;

            timeserver_save_timezone ();
        }

        rtc = 1;
    }
    return rtc;
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * set new timeserver
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
timeserver_set_timeserver (char * new_timeserver)
{
    uint_fast8_t    rtc;

    strncpy ((char *) timeserver.timeserver, new_timeserver, EEPROM_DATA_SIZE_TIMESERVER - 1);
    timeserver.timeserver[EEPROM_DATA_SIZE_TIMESERVER - 1] = '\0';
    rtc = timeserver_save_timeserver ();
    return rtc;
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * start timeserver request
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
void
timeserver_start_timeserver_request (void)
{
    if (timeserver.timeserver && *timeserver.timeserver)
    {
        esp8266_send_cmd ("time", timeserver.timeserver, 1);
    }
    else
    {
        esp8266_send_cmd ("time", (const char *) NULL, 1);                              // use default timeserver
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * check net time (A62, specs/paket-2026-10-11, design.md 2, Ent-3 und Ent-3b)
 *
 * Prueft den Text einer TIME-Zeile vom ESP, bevor irgendetwas an Uhr oder RTC geaendert wird.
 *
 * 1. Form: 1 bis 10 Ziffern, dann '\0'. Kein Vorzeichen, kein Leerzeichen, kein Rest; ein Wert ueber
 *    4'294'967'295 ist ungueltig (strtoul saettigte dort still). 1 bis 10 und nicht genau 10: Der ESP
 *    sendet die NTP-Sekunde dezimal OHNE fuehrende Nullen (ntp.cpp), und ab 2036-02-07 06:28:16 UTC
 *    (Aera 1) beginnt sie wieder bei 0.
 *
 * 2. Gueltigkeit -- EINE Grenze auf der Unixzeit: (uint32_t) (n - 2208988800) >= 1767225600, also ab
 *    2026-01-01 00:00:00 UTC. Die Differenz laeuft modulo 2^32 um und bildet damit Aera 0 und Aera 1
 *    in einem Schritt ab, dieselbe Rechnung wie in timeserver_convert_time(); kein 64-Bit-Pfad.
 *    Gueltig ist 2026-01-01 bis 2106-02-07 06:28:15 UTC (uint32 in my_gmtime()); verworfen wird der
 *    Totbereich n in [2208988800, 3976214400). Die Kette endet trotzdem frueher: Die RTC speichert das
 *    Jahr zweistellig, ab 2100 schreibt rtc.c ein ungueltiges Jahr (L371).
 *
 * 3. Bestaetigung (Ent-3b): Weicht n um mehr als 600 s von der letzten uebernommenen, ueber uptime
 *    fortgeschriebenen NTP-Sekunde ab, gilt n erst, wenn eine zweite Antwort es auf +-5 s bestaetigt --
 *    und zwar eine SPAETERE, mindestens 10 s nach dem Kandidaten. Ohne diesen Abstand bestaetigten
 *    sich zwei Nullen, die kurz nacheinander ankommen (zwei Anfragen in derselben Sekunde).
 *    Nach jedem Start gibt es keine Referenz (RAM): Die erste Antwort wird Kandidat, die Uhr laeuft bis
 *    zur Bestaetigung aus der RTC. Faengt die 0 eines unsynchronisierten Servers (0 und 0 bestaetigen
 *    sich nicht, erwartet waere 0 + 15) und eine verstuemmelte Zeile -- TIME traegt keine Pruefsumme.
 *    Die Referenz ist UTC; Zeitzone und Sommerzeit wirken erst in der Umrechnung. Alle Abstaende sind
 *    uint32-Differenzen: Sie ueberstehen die Aerengrenze und den Umlauf von uptime.
 *
 * Rueckgabe: TIMESERVER_NET_TIME_OK         uebernehmen, *np gesetzt
 *            TIMESERVER_NET_TIME_REJECTED   nichts aendern
 *            TIMESERVER_NET_TIME_ASK_AGAIN  nichts aendern, in 15 s nochmals fragen -- hoechstens dreimal
 *                                           in Folge, danach der regulaere Takt (keine Abfrageschleife)
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static struct
{
    uint32_t        ref;                                                    // letzte uebernommene NTP-Sekunde
    uint32_t        ref_up;                                                 // uptime dazu
    uint32_t        cand;                                                   // Kandidat, wartet auf Bestaetigung
    uint32_t        cand_up;                                                // uptime dazu
    uint8_t         ref_ok;
    uint8_t         cand_ok;
    uint8_t         quick;                                                  // Schnellabfragen in Folge, 0..3
} net_time;

uint_fast8_t
timeserver_check_net_time (const char * s, uint32_t up, uint32_t * np)
{
    uint32_t        n = 0;
    uint_fast8_t    i;

    for (i = 0; s[i] >= '0' && s[i] <= '9'; i++)
    {
        uint_fast8_t d = s[i] - '0';

        if (i == 10 || n > 429496729U || (n == 429496729U && d > 5))      // 11. Ziffer oder ueber 4'294'967'295
        {
            i = 0;
            break;
        }

        n = n * 10 + d;
    }

    if (i == 0 || s[i] != '\0' || n - 2208988800U < 1767225600U)            // Form; Totbereich (Unix vor 2026-01-01)
    {
        log_message ("time: ungueltig, verworfen");
        return TIMESERVER_NET_TIME_REJECTED;
    }

    if ((net_time.ref_ok  && (uint32_t) (n - (net_time.ref + (up - net_time.ref_up)) + 600) <= 1200) ||
        (net_time.cand_ok && up - net_time.cand_up >= 10 &&
                             (uint32_t) (n - (net_time.cand + (up - net_time.cand_up)) + 5) <= 10))
    {                                                                       // regulaerer Abgleich +-600 s oder bestaetigt +-5 s
        net_time.ref     = n;
        net_time.ref_up  = up;
        net_time.ref_ok  = 1;
        net_time.cand_ok = 0;
        net_time.quick   = 0;
        *np = n;
        return TIMESERVER_NET_TIME_OK;
    }

    net_time.cand    = n;
    net_time.cand_up = up;
    net_time.cand_ok = 1;
    log_message ("time: sprung, bestaetigung angefordert");

    if (net_time.quick < 3)
    {
        net_time.quick++;
        return TIMESERVER_NET_TIME_ASK_AGAIN;
    }

    return TIMESERVER_NET_TIME_REJECTED;
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * convert time
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
void
timeserver_convert_time (struct tm * tmp, uint32_t seconds_since_1900)
{
    time_t          curtime;
    struct tm *     mytm;

    curtime = (time_t) (seconds_since_1900 - 2208988800U);
    curtime += 3600 * timeserver.timezone;                                  // add seconds for time zone (e.g. MEZ: +3600)

    mytm = my_gmtime (&curtime);                                            // localtime() needs 4K more flash, but does return same as gmtime()

    if (timeserver.observe_summertime)
    {
        uint_fast8_t    summertime = 0;

        // calculate summer time:
        if (mytm->tm_mon >= 3 && mytm->tm_mon <= 8)                         // april to september
        {
            summertime = 1;
        }
        else if (mytm->tm_mon == 2)                                         // march
        {
            if (mytm->tm_mday - mytm->tm_wday >= 25)                        // after or equal last sunday in march
            {
                if (mytm->tm_wday == 0)                                     // today last sunday?
                {
                    if (mytm->tm_hour >= 2)                                 // after 02:00 we have summer time
                    {
                        summertime = 1;
                    }
                }
                else
                {
                    summertime = 1;
                }
            }
        }
        else if (mytm->tm_mon == 9)                                         // it's october
        {
            summertime = 1;

            if (mytm->tm_mday - mytm->tm_wday >= 25)                        // it's after or equal last sunday in october...
            {
                if (mytm->tm_wday == 0)                                     // today last sunday?
                {
                    if (mytm->tm_hour >= 2)                                 // after 02:00 standard time basis we have winter time
                    {
                        summertime = 0;
                    }
                }
                else
                {
                    summertime = 0;
                }
            }
        }

        if (summertime)
        {
            curtime += 3600;                                                // add one hour more for MESZ
            mytm = my_gmtime (&curtime);                                    // localtime() needs 4K more flash, but does return same as gmtime()
        }
    }

    *tmp = *mytm;
}
