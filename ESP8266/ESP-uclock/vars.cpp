/*----------------------------------------------------------------------------------------------------------------------------------------
 * vars.cpp - synchronization of variables between STM32 and ESP8266
 *
 * Copyright (c) 2016-2025 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#include "Arduino.h"
#include "base.h"
#include "vars.h"
#include "version.h"

#define CMD_CODE_NUMERIC_VAR                            'N'                        // command:   numeric variable
#define CMD_CODE_NUMERIC_ARRAY                          'n'                        // command:   numeric array
#define CMD_CODE_STRING_VAR                             'S'                        // command:   string variable

#define CMD_CODE_TIME_VAR                               'T'                        // command:   time variable

#define CMD_CODE_DISPLAY_VAR                            'D'                        // command:   display variable
#define PAR_CODE_DISPLAY_COLOR                          'C'                        // parameter: display color

#define CMD_CODE_ANIMATION_VAR                          'A'                        // command:   animation variable
#define PAR_CODE_ANIMATION_MODE_NAME                    'N'                        // parameter: animation mode name
#define PAR_CODE_ANIMATION_DECELERATION                 'D'                        // parameter: animation deceleration
#define PAR_CODE_ANIMATION_DEFAULT_DECELERATION         'E'                        // parameter: animation default deceleration
#define PAR_CODE_ANIMATION_FLAGS                        'F'                        // parameter: animation flags

#define CMD_CODE_COLOR_ANIMATION_VAR                    'C'                        // command:   color animation variable
#define PAR_CODE_COLOR_ANIMATION_MODE_NAME              'N'                        // parameter: animation mode name
#define PAR_CODE_COLOR_ANIMATION_DECELERATION           'D'                        // parameter: animation deceleration
#define PAR_CODE_COLOR_ANIMATION_DEFAULT_DECELERATION   'E'                        // parameter: animation default deceleration
#define PAR_CODE_COLOR_ANIMATION_FLAGS                  'F'                        // parameter: animation flags

#define CMD_CODE_AMBILIGHT_MODE_VAR                     'M'                        // command:   ambilight mode variable
#define PAR_CODE_AMBILIGHT_MODE_NAME                    'N'                        // parameter: animation mode name
#define PAR_CODE_AMBILIGHT_MODE_DECELERATION            'D'                        // parameter: animation deceleration
#define PAR_CODE_AMBILIGHT_MODE_DEFAULT_DECELERATION    'E'                        // parameter: animation default deceleration
#define PAR_CODE_AMBILIGHT_MODE_FLAGS                   'F'                        // parameter: animation flags

#define CMD_CODE_OVERLAY_VAR                            'O'                        // command:   overlay variable
#define PAR_CODE_OVERLAY_TYPE                           'T'                        // parameter: overlay type
#define PAR_CODE_OVERLAY_INTERVAL                       'I'                        // parameter: overlay interval
#define PAR_CODE_OVERLAY_DURATION                       'D'                        // parameter: overlay duration
#define PAR_CODE_OVERLAY_DATE_CODE                      'C'                        // parameter: overlay date code
#define PAR_CODE_OVERLAY_DATE_START                     'S'                        // parameter: overlay date start
#define PAR_CODE_OVERLAY_DAYS                           'Y'                        // parameter: overlay days
#define PAR_CODE_OVERLAY_TEXT                           'N'                        // parameter: overlay name or text
#define PAR_CODE_OVERLAY_FLAGS                          'F'                        // parameter: overlay flags

#define CMD_CODE_NIGHT_TIME_TABLE                       't'                        // command:   night time table
#define CMD_CODE_AMBILIGHT_NIGHT_TIME_TABLE             'a'                        // command:   ambilight night time table

#define CMD_CODE_ALARM_TIME_TABLE                       'l'                        // command:   alarm time table
#define CMD_CODE_IR_CODE                                'I'                        // command:   ir code (both directions)

unsigned int
rpc (RPC_VARIABLE var)
{
    unsigned int   rtc = 0;

    if (var < MAX_RPC_VARIABLES)
    {
        Serial.printf ("CMD R%02x\r\n", (int) var);
        Serial.flush ();
        rtc = 1;
    }

    return rtc;
}

unsigned int     numvars[MAX_NUM_VARIABLES];

unsigned int
get_numvar (NUM_VARIABLE var)
{
    unsigned int   rtc = 0;

    if (var < MAX_NUM_VARIABLES)
    {
        rtc =  numvars[var];
    }

    return rtc;
}

unsigned int
set_numvar (NUM_VARIABLE var, unsigned int value)
{
    unsigned int   rtc = 0;

    if (var < MAX_NUM_VARIABLES)
    {
        numvars[var] = value;
        Serial.printf ("CMD N%02x%02x%02x\r\n", (int) var, value & 0xFF, (value >> 8) & 0xFF);
        Serial.flush ();
        rtc = 1;
    }

    return rtc;
}

uint8_t   max_display_animation_variables = 0;
uint8_t   dimmed_display_colors[MAX_BRIGHTNESS + 1];
uint8_t   dimmed_ambilight_colors[MAX_BRIGHTNESS + 1];

uint_fast8_t
get_num8_array (NUM8_ARRAY var, uint32_t idx)
{
    uint_fast8_t  rtc = 0;

    switch (var)
    {
        case DISPLAY_DIMMED_DISPLAY_COLORS:
        {
            if (idx < MAX_BRIGHTNESS + 1)
            {
                rtc = dimmed_display_colors[idx];
            }
            break;
        }
        case DISPLAY_DIMMED_AMBILIGHT_COLORS:
        {
            if (idx < MAX_BRIGHTNESS + 1)
            {
                rtc = dimmed_ambilight_colors[idx];
            }
            break;
        }
    }

    return rtc;
}

uint_fast8_t
set_num8_array (NUM8_ARRAY var, uint32_t idx, uint_fast8_t value)
{
    uint_fast8_t  rtc = 0;

    switch (var)
    {
        case DISPLAY_DIMMED_DISPLAY_COLORS:
        {
            if (idx < MAX_BRIGHTNESS + 1)
            {
                dimmed_display_colors[idx] = value;
                Serial.printf ("CMD n%02x%02x%02x\r\n", (int) var, idx, value);
                Serial.flush ();
                rtc = 1;
            }
            break;
        }
        case DISPLAY_DIMMED_AMBILIGHT_COLORS:
        {
            if (idx < MAX_BRIGHTNESS + 1)
            {
                dimmed_ambilight_colors[idx] = value;
                Serial.printf ("CMD n%02x%02x%02x\r\n", (int) var, idx, value);
                Serial.flush ();
                rtc = 1;
            }
            break;
        }
    }

    return rtc;
}

#if 0 // uint16_t arrays, yet not used

uint_fast16_t
get_num16_array (NUM16_ARRAY var, uint32_t idx)
{
    uint_fast16_t  rtc = 0;

    switch (var)
    {
        case FOO_ARRAY:
            if (idx < FOO_ARRAY_ENTRIES)
            {
                rtc = foo_array[idx];
            }
            break;
    }

    return rtc;
}

uint_fast8_t
set_num16_array (NUM16_ARRAY var, uint32_t idx, uint_fast16_t value)
{
    uint_fast8_t  rtc = 0;

    switch (var)
    {
        case DFPLAYER_PLAYLIST_ARRAY:
            if (idx < MAX_PLAYLIST_TRACKS)
            {
                dfplayer_playlist[idx] = value;
                Serial.printf ("CMD m%02x%02x%02x%02x\r\n", (int) var, idx, value & 0xFF, (value >> 8) & 0xFF);
                Serial.flush ();
                rtc = 1;
            }
    }

    return rtc;
}

#endif // 0

static char ticker_text[MAX_TICKER_TEXT_LEN + 1];
static char version[MAX_VERSION_TEXT_LEN + 1];
static char eeprom_version[MAX_EEPROM_VERSION_TEXT_LEN + 1];
static char esp8266_version[MAX_ESP8266_VERSION_TEXT_LEN + 1];
static char timeserver_name[MAX_TIMESERVER_NAME_LEN + 1];
static char weather_appid[MAX_WEATHER_APPID_LEN + 1];
static char weather_city[MAX_WEATHER_CITY_LEN + 1];
static char weather_lon[MAX_WEATHER_LON_LEN + 1];
static char weather_lat[MAX_WEATHER_LAT_LEN + 1];
static char update_host[MAX_UPDATE_HOST_LEN + 1];
static char update_path[MAX_UPDATE_PATH_LEN + 1];
static char date_ticker_format[MAX_DATE_TICKER_FORMAT_LEN + 1];
static char reset_cause[MAX_RESET_CAUSE_LEN + 1];

STR_VAR strvars[MAX_STR_VARIABLES] =
{
    { ticker_text,          MAX_TICKER_TEXT_LEN },
    { version,              MAX_VERSION_TEXT_LEN },
    { eeprom_version,       MAX_EEPROM_VERSION_TEXT_LEN },
    { esp8266_version,      MAX_ESP8266_VERSION_TEXT_LEN },
    { timeserver_name,      MAX_TIMESERVER_NAME_LEN },
    { weather_appid,        MAX_WEATHER_APPID_LEN },
    { weather_city,         MAX_WEATHER_CITY_LEN },
    { weather_lon,          MAX_WEATHER_LON_LEN },
    { weather_lat,          MAX_WEATHER_LAT_LEN },
    { update_host,          MAX_UPDATE_HOST_LEN },
    { update_path,          MAX_UPDATE_PATH_LEN },
    { date_ticker_format,   MAX_DATE_TICKER_FORMAT_LEN },
    { reset_cause,          MAX_RESET_CAUSE_LEN },
};


STR_VAR *
get_strvar (STR_VARIABLE var)
{
    STR_VAR *   rtc = (STR_VAR *) 0;

    if (var < MAX_STR_VARIABLES)
    {
        rtc = &(strvars[var]);
    }

    return rtc;
}

/* Die Maximallaengen sind in BYTES angegeben, die Oberflaeche zaehlt aber ZEICHEN
 * (maxlength="32"). Ein Text, der an der Grenze mitten in einem UTF-8-Mehrbytezeichen
 * endet, hinterliess bisher ein halbes Zeichen im Wert - die settings_xml wurde damit
 * unlesbar und der Wert ueber die PWA nicht mehr korrigierbar (L46).
 * Deshalb: auf die naechste Zeichengrenze zurueckgehen und ein angefangenes Zeichen
 * verwerfen. Liefert die Zahl der zu uebernehmenden Bytes.
 */
unsigned int
utf8_truncated_len (const char * p, unsigned int maxlen)
{
    unsigned int len;
    unsigned int cut;

    if (! p)
    {
        return 0;
    }

    len = strlen (p);

    if (len <= maxlen)
    {
        return len;
    }

    cut = maxlen;

    while (cut > 0 && ((unsigned char) p[cut] & 0xC0) == 0x80)          // erstes verworfenes Byte ist Folgebyte
    {
        cut--;
    }

    if (cut > 0 && ((unsigned char) p[cut - 1] & 0xC0) == 0xC0)         // letztes behaltenes Byte ist ein Startbyte ohne Fortsetzung
    {
        cut--;
    }

    return cut;
}

/* Kopiert hoechstens maxlen Bytes und endet dabei immer auf einer Zeichengrenze (L46). */
void
utf8_copy_truncated (char * dst, const char * src, unsigned int maxlen)
{
    unsigned int len = utf8_truncated_len (src, maxlen);

    if (len > 0)
    {
        memcpy (dst, src, len);
    }

    dst[len] = '\0';
}

unsigned int
set_strvar (STR_VARIABLE var, const char * p)
{
    unsigned int   rtc = 0;

    if (var < MAX_STR_VARIABLES)
    {
        memset (strvars[var].str, 0, strvars[var].maxlen + 1);
        utf8_copy_truncated (strvars[var].str, p ? p : "", strvars[var].maxlen);
        /* Der STM32 bekommt denselben gekuerzten Wert - sonst schneidet er mit seinen
         * eigenen Grenzen erneut und erzeugt genau das halbe Zeichen wieder (L46).
         */
        Serial.printf ("CMD S%02x%s\r\n", (int) var, strvars[var].str);
        Serial.flush ();
        rtc =  1;
    }

    return rtc;
}

TM tmvars[MAX_TM_VARIABLES];

TM *
get_tm_var (TM_VARIABLE var)
{
    TM *   rtc = (TM *) 0;

    if (var < MAX_TM_VARIABLES)
    {
        rtc = &(tmvars[var]);
    }

    return rtc;

}

unsigned int
set_tm_var (TM_VARIABLE var, TM * tm)
{
    unsigned int   rtc = 0;

    if (var < MAX_TM_VARIABLES)
    {
        memcpy (&tmvars[var], tm, sizeof (TM));
        Serial.printf ("CMD T%02x%04d%02d%02d%02d%02d%02d\r\n", (int) var, tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        Serial.flush ();
        rtc =  1;
    }

    return rtc;
}

DSP_COLORS dspcolorvars[MAX_DSP_COLOR_VARIABLES];

unsigned int
get_dsp_color_var (DSP_COLOR_VARIABLE var, DSP_COLORS * t)
{
    unsigned int   rtc = 0;

    if (var < MAX_DSP_COLOR_VARIABLES)
    {
        memcpy (t, &(dspcolorvars[var]), sizeof (DSP_COLORS));
        rtc =  1;
    }

    return rtc;
}

unsigned int
set_dsp_color_var (DSP_COLOR_VARIABLE var, DSP_COLORS * s, int use_rgbw)
{
    unsigned int   rtc = 0;

    if (var < MAX_DSP_COLOR_VARIABLES)
    {
        memcpy (&(dspcolorvars[var]), s, sizeof (DSP_COLORS));

        if (use_rgbw)
        {
            Serial.printf ("CMD DC%02x%02x%02x%02x%02x\r\n", (int) var, s->red, s->green, s->blue, s->white);
        }
        else
        {
            Serial.printf ("CMD DC%02x%02x%02x%02x\r\n", (int) var, s->red, s->green, s->blue);
        }

        Serial.flush ();
        rtc = 1;
    }

    return rtc;
}

DISPLAY_ANIMATION displayanimationvars[MAX_DISPLAY_ANIMATION_VARIABLES];

DISPLAY_ANIMATION *
get_display_animation_var (uint_fast8_t var)
{
    DISPLAY_ANIMATION *   rtc = (DISPLAY_ANIMATION *) 0;

    if (var < MAX_DISPLAY_ANIMATION_VARIABLES)
    {
        rtc = &(displayanimationvars[var]);
    }

    return rtc;
}

unsigned int
set_display_animation_name (uint_fast8_t var, char * name)
{
    unsigned int   rtc = 0;

    if (var < MAX_DISPLAY_ANIMATION_VARIABLES)
    {
        strncpy (displayanimationvars[var].name, name, MAX_DISPLAY_ANIMATION_NAME_LEN);
        Serial.printf ("CMD AN%02x%s\r\n", (int) var, name);
        Serial.flush ();
        rtc =  1;
    }

    return rtc;
}

unsigned int
set_display_animation_deceleration (uint_fast8_t var, uint_fast8_t deceleration)
{
    unsigned int   rtc = 0;

    if (var < MAX_DISPLAY_ANIMATION_VARIABLES)
    {
        displayanimationvars[var].deceleration = deceleration;
        Serial.printf ("CMD AD%02x%02x\r\n", (int) var, deceleration);
        Serial.flush ();
        rtc =  1;
    }

    return rtc;
}

unsigned int
set_display_animation_default_deceleration (uint_fast8_t var, uint_fast8_t default_deceleration)
{
    unsigned int   rtc = 0;

    if (var < MAX_DISPLAY_ANIMATION_VARIABLES)
    {
        displayanimationvars[var].default_deceleration = default_deceleration;
        Serial.printf ("CMD AE%02x%02x\r\n", (int) var, default_deceleration);
        Serial.flush ();
        rtc =  1;
    }

    return rtc;
}

unsigned int
set_display_animation_flags (uint_fast8_t var, uint_fast8_t flags)
{
    unsigned int   rtc = 0;

    if (var < MAX_DISPLAY_ANIMATION_VARIABLES)
    {
        displayanimationvars[var].flags = flags;
        Serial.printf ("CMD AF%02x%02x\r\n", (int) var, flags);
        Serial.flush ();
        rtc =  1;
    }

    return rtc;
}

COLOR_ANIMATION coloranimationvars[MAX_COLOR_ANIMATION_VARIABLES];

COLOR_ANIMATION *
get_color_animation_var (COLOR_ANIMATION_VARIABLE var)
{
    COLOR_ANIMATION *   rtc = (COLOR_ANIMATION *) 0;

    if (var < MAX_COLOR_ANIMATION_VARIABLES)
    {
        rtc = &(coloranimationvars[var]);
    }

    return rtc;
}

unsigned int
set_color_animation_name (COLOR_ANIMATION_VARIABLE var, char * name)
{
    unsigned int   rtc = 0;

    if (var < MAX_COLOR_ANIMATION_VARIABLES)
    {
        strncpy (coloranimationvars[var].name, name, MAX_COLOR_ANIMATION_NAME_LEN);
        Serial.printf ("CMD CN%02x%s\r\n", (int) var, name);
        Serial.flush ();
        rtc =  1;
    }

    return rtc;
}

unsigned int
set_color_animation_deceleration (COLOR_ANIMATION_VARIABLE var, uint_fast8_t deceleration)
{
    unsigned int   rtc = 0;

    if (var < MAX_COLOR_ANIMATION_VARIABLES)
    {
        coloranimationvars[var].deceleration = deceleration;
        Serial.printf ("CMD CD%02x%02x\r\n", (int) var, deceleration);
        Serial.flush ();
        rtc =  1;
    }

    return rtc;
}

unsigned int
set_color_animation_default_deceleration (COLOR_ANIMATION_VARIABLE var, uint_fast8_t default_deceleration)
{
    unsigned int   rtc = 0;

    if (var < MAX_COLOR_ANIMATION_VARIABLES)
    {
        coloranimationvars[var].default_deceleration = default_deceleration;
        Serial.printf ("CMD CE%02x%02x\r\n", (int) var, default_deceleration);
        Serial.flush ();
        rtc =  1;
    }

    return rtc;
}

unsigned int
set_color_animation_flags (COLOR_ANIMATION_VARIABLE var, uint_fast8_t flags)
{
    unsigned int   rtc = 0;

    if (var < MAX_COLOR_ANIMATION_VARIABLES)
    {
        coloranimationvars[var].flags = flags;
        Serial.printf ("CMD CF%02x%02x\r\n", (int) var, flags);
        Serial.flush ();
        rtc =  1;
    }

    return rtc;
}

AMBILIGHT_MODE ambilightmodevars[MAX_AMBILIGHT_MODE_VARIABLES];

AMBILIGHT_MODE *
get_ambilight_mode_var (AMBILIGHT_MODE_VARIABLE var)
{
    AMBILIGHT_MODE *   rtc = (AMBILIGHT_MODE *) 0;

    if (var < MAX_AMBILIGHT_MODE_VARIABLES)
    {
        rtc = &(ambilightmodevars[var]);
    }

    return rtc;
}

unsigned int
set_ambilight_mode_name (AMBILIGHT_MODE_VARIABLE var, char * name)
{
    unsigned int   rtc = 0;

    if (var < MAX_AMBILIGHT_MODE_VARIABLES)
    {
        strncpy (ambilightmodevars[var].name, name, MAX_AMBILIGHT_MODE_VARIABLES);
        Serial.printf ("CMD MN%02x%s\r\n", (int) var, name);
        Serial.flush ();
        rtc =  1;
    }

    return rtc;
}

unsigned int
set_ambilight_mode_deceleration (AMBILIGHT_MODE_VARIABLE var, uint_fast8_t deceleration)
{
    unsigned int   rtc = 0;

    if (var < MAX_AMBILIGHT_MODE_VARIABLES)
    {
        ambilightmodevars[var].deceleration = deceleration;
        Serial.printf ("CMD MD%02x%02x\r\n", (int) var, deceleration);
        Serial.flush ();
        rtc =  1;
    }

    return rtc;
}

unsigned int
set_ambilight_mode_default_deceleration (AMBILIGHT_MODE_VARIABLE var, uint_fast8_t default_deceleration)
{
    unsigned int   rtc = 0;

    if (var < MAX_AMBILIGHT_MODE_VARIABLES)
    {
        ambilightmodevars[var].default_deceleration = default_deceleration;
        Serial.printf ("CMD ME%02x%02x\r\n", (int) var, default_deceleration);
        Serial.flush ();
        rtc =  1;
    }

    return rtc;
}

unsigned int
set_ambilight_mode_flags (AMBILIGHT_MODE_VARIABLE var, uint_fast8_t flags)
{
    unsigned int   rtc = 0;

    if (var < MAX_AMBILIGHT_MODE_VARIABLES)
    {
        ambilightmodevars[var].flags = flags;
        Serial.printf ("CMD MF%02x%02x\r\n", (int) var, flags);
        Serial.flush ();
        rtc =  1;
    }

    return rtc;
}

/* %02x ist eine MINDESTbreite, keine Hoechstbreite. Der STM liest dagegen mit FESTER
 * Breite (htoi (parameters, 2), src/main.c:2405-2455): Ein Wert ueber 255 erzeugt drei
 * Hexziffern, und der STM nimmt davon nur die ersten beiden. date_code=300 wurde so zu
 * "12c" und kam als 0x12 = 18 an - der STM meldete "invalid date_code: 18" (L66).
 * Jeder andere Mehrbyte-Sender in dieser Datei maskiert laengst (set_night_time_var,
 * set_numvar, set_num8_array); hier fehlte die Maske als einzige.
 *
 * Die Maske ist nur das Netz. Geprueft wird im Endpunkt (http_api_overlay_set), denn
 * eine Maske allein wuerde aus 300 still eine 44 machen und damit etwas anderes
 * speichern, als der Aufrufer wollte.
 */
unsigned int
set_overlay_var (uint_fast8_t idx)
{
    Serial.printf ("CMD OT%02x%02x\r\n", idx & 0xFF, overlays[idx].type & 0xFF);
    Serial.printf ("CMD OI%02x%02x\r\n", idx & 0xFF, overlays[idx].interval & 0xFF);
    Serial.printf ("CMD OD%02x%02x\r\n", idx & 0xFF, overlays[idx].duration & 0xFF);
    Serial.printf ("CMD OC%02x%02x\r\n", idx & 0xFF, overlays[idx].date_code & 0xFF);
    Serial.printf ("CMD OS%02x%04x\r\n", idx & 0xFF, overlays[idx].date_start & 0xFFFF);
    Serial.printf ("CMD OY%02x%02x\r\n", idx & 0xFF, overlays[idx].days & 0xFF);
    Serial.printf ("CMD ON%02x%s\r\n",   idx & 0xFF, overlays[idx].text);
    Serial.printf ("CMD OF%02x%02x\r\n", idx & 0xFF, overlays[idx].flags & 0xFF);
    Serial.flush ();

    return 1;
}

NIGHT_TIME nighttimevars[MAX_NIGHT_TIME_VARIABLES];
NIGHT_TIME ambilightnighttimevars[MAX_NIGHT_TIME_VARIABLES];

NIGHT_TIME *
get_night_time_var (uint_fast8_t is_ambilight, NIGHT_TIME_VARIABLE var)
{
    NIGHT_TIME *   rtc = (NIGHT_TIME *) 0;

    if (var < MAX_NIGHT_TIME_VARIABLES)
    {
        if (is_ambilight)
        {
            rtc = &(ambilightnighttimevars[var]);
        }
        else
        {
            rtc = &(nighttimevars[var]);
        }
    }

    return rtc;
}

unsigned int
set_night_time_var (uint_fast8_t is_ambilight, NIGHT_TIME_VARIABLE var, uint_fast16_t minutes, uint_fast8_t flags)
{
    unsigned int   rtc = 0;

    if (var < MAX_NIGHT_TIME_VARIABLES)
    {
        if (is_ambilight)
        {
            ambilightnighttimevars[var].minutes = minutes;
            ambilightnighttimevars[var].flags = flags;
            Serial.printf ("CMD a%02x%02x%02x%02x\r\n", (int) var, minutes & 0xFF, (minutes >> 8) & 0xFF, flags);
        }
        else
        {
            nighttimevars[var].minutes = minutes;
            nighttimevars[var].flags = flags;
            Serial.printf ("CMD t%02x%02x%02x%02x\r\n", (int) var, minutes & 0xFF, (minutes >> 8) & 0xFF, flags);
        }
        Serial.flush ();
        rtc =  1;
    }

    return rtc;
}

ALARM_TIME alarmtimevars[MAX_ALARM_TIME_VARIABLES];

ALARM_TIME *
get_alarm_time_var (ALARM_TIME_VARIABLE var)
{
    ALARM_TIME *   rtc = (ALARM_TIME *) 0;

    if (var < MAX_ALARM_TIME_VARIABLES)
    {
        rtc = &(alarmtimevars[var]);
    }

    return rtc;
}

unsigned int
set_alarm_time_var (ALARM_TIME_VARIABLE var, uint_fast16_t minutes, uint_fast8_t flags)
{
    unsigned int   rtc = 0;

    if (var < MAX_ALARM_TIME_VARIABLES)
    {
        alarmtimevars[var].minutes = minutes;
        alarmtimevars[var].flags = flags;
        Serial.printf ("CMD l%02x%02x%02x%02x\r\n", (int) var, minutes & 0xFF, (minutes >> 8) & 0xFF, flags);
        Serial.flush ();
        rtc =  1;
    }

    return rtc;
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * IR-Codes: fluechtiger Spiegel der angelernten Fernbedienungstasten des STM32
 *
 * Der Puffer liegt ausschliesslich im RAM und wird bewusst NICHT ins ESP-EEPROM gespiegelt. Nach einem
 * ESP-Neustart ist er leer und ir_codes_requested 0; die PWA bricht daran erkennbar ab, statt einen
 * leeren Satz als Sicherung zu schreiben. Ein Abzug mit 20 leeren Tasten, der aussieht wie ein gueltiges
 * Backup, waere schlimmer als gar keiner.
 *
 * Vollstaendigkeit wird gezaehlt, nicht geschaetzt: ir_codes_received_mask traegt ein Bit je Taste,
 * ir_codes_is_complete() ist exakt der Vergleich gegen IR_CODES_COMPLETE_MASK. Welche Indizes fehlen,
 * ist aus der Maske ableitbar -- das ist der Zweck von missing[] im Endpunkt.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static IR_CODE      ir_codes[MAX_IR_CODES];
static uint32_t     ir_codes_received_mask;                                // Bit i gesetzt == Taste i eingetroffen
static uint8_t      ir_codes_requested;                                    // 1 == seit dem letzten Anstoss gueltig

void
ir_codes_begin_request (void)
{
    memset (ir_codes, 0, sizeof (ir_codes));
    ir_codes_received_mask  = 0;
    ir_codes_requested      = 1;
}

/* Entwertet den Puffer, statt ihn mitzufuehren. Das ist Absicht und nicht vergessen: Auf einem Geraet
 * ohne Fernbedienung ist der Abzug der einzige von aussen sichtbare Zustand. Ein Puffer, der die eigene
 * Eingabe zurueckspiegelt, macht die einzige verfuegbare Gegenprobe wertlos -- man pruefte dann, ob der
 * ESP sich merkt, was man ihm gesagt hat, statt ob der STM es gespeichert hat.
 */
void
ir_codes_invalidate (void)
{
    ir_codes_received_mask  = 0;
    ir_codes_requested      = 0;
}

uint_fast8_t
ir_codes_are_requested (void)
{
    return ir_codes_requested;
}

uint32_t
ir_codes_mask (void)
{
    return ir_codes_received_mask;
}

uint_fast8_t
ir_codes_count (void)
{
    uint_fast8_t    idx;
    uint_fast8_t    n = 0;

    for (idx = 0; idx < MAX_IR_CODES; idx++)
    {
        if (ir_codes_received_mask & (((uint32_t) 1) << idx))
        {
            n++;
        }
    }

    return n;
}

uint_fast8_t
ir_codes_is_complete (void)
{
    return (ir_codes_received_mask == IR_CODES_COMPLETE_MASK) ? 1 : 0;
}

/* Liefert nur eingetroffene Tasten. Ein Index, dessen Bit fehlt, gibt 0 zurueck statt eines Nullwerts,
 * der sich von einer nie angelernten Taste nicht unterscheiden liesse.
 */
IR_CODE *
get_ir_code (uint_fast8_t idx)
{
    IR_CODE *   rtc = (IR_CODE *) 0;

    if (idx < MAX_IR_CODES && (ir_codes_received_mask & (((uint32_t) 1) << idx)))
    {
        rtc = &(ir_codes[idx]);
    }

    return rtc;
}

/* Maskiert wie set_overlay_var und set_night_time_var: %02x ist eine MINDESTbreite, der STM liest
 * dagegen mit FESTER Breite (htoi (parameters, 2) bzw. 4, src/main.c:2052-2061). Ein Wert ueber 255
 * erzeugt sonst drei Hexziffern, und der Empfaenger nimmt davon die ersten beiden -- genau Befund L66.
 * Die Maske ist nur das Netz; geprueft werden die Werte im Endpunkt.
 */
unsigned int
set_ir_code_var (uint_fast8_t idx, uint_fast8_t protocol, uint_fast16_t address, uint_fast16_t command)
{
    unsigned int   rtc = 0;

    if (idx < MAX_IR_CODES)
    {
        Serial.printf ("CMD I%02x%02x%04x%04x\r\n",
                       (unsigned int) (idx & 0xFF),
                       (unsigned int) (protocol & 0xFF),
                       (unsigned int) (address & 0xFFFF),
                       (unsigned int) (command & 0xFFFF));
        Serial.flush ();
        ir_codes_invalidate ();
        rtc = 1;
    }

    return rtc;
}

OVERLAY      overlays[MAX_OVERLAYS];

/*----------------------------------------------------------------------------------------------------------------------------------------
 * var_cmd_min_len () - Mindestlaenge einer Kommandozeile, Befund L237
 *
 * Hintergrund: In var_set_parameter() folgt auf jedes htoi (parameters, 2) ein UNBEDINGTES
 * parameters += 2 -- an rund 45 Stellen nach demselben Bauplan. Stand der Zeiger bereits auf
 * dem Terminator, zeigt er danach DAHINTER, und der naechste htoi liest dort.
 *
 * Der Fix aus L232 begrenzt den Schaden -- htoi haelt am ersten Nullbyte --, hebt ihn aber
 * nicht auf: Hinter dem Terminator steht nicht zwingend eine weitere Null. Sie steht dort
 * sogar meistens NICHT. Die Zeile liegt in cmd_buffer, einem static char [CMD_BUFFER_SIZE]
 * im Hauptloop (ESP-uclock.ino:499). Der wird nie geleert, nur an cmd_len terminiert --
 * dahinter liegt der Rest des VORIGEN Kommandos. Eine um ein Zeichen verkuerzte Zeile liest
 * also Text aus dem Kommando davor und macht daraus einen gueltig aussehenden falschen Wert.
 *
 * Nachgerechnet und am Pruefstand nachgestellt: In cmd_buffer steht noch "var ON00Weihnachten"
 * (Index 8 = 'W', 9 = 'e'). Danach trifft statt "var OT0002" nur "var OT0" ein -- drei Zeichen
 * auf der Bruecke verloren. Der Terminator liegt jetzt auf Index 7, die Buchstaben ab Index 8
 * stehen unveraendert aus dem vorigen Kommando da. Ablauf: cmd_code = 'O' (Index 4),
 * cmd_code = 'T' (Index 5), var_idx = htoi (Index 6, 2) = 0 -- haelt am Terminator, also noch
 * harmlos --, dann parameters += 2 UNBEDINGT, und der Zeiger steht auf Index 8, also HINTER
 * dem Terminator. type = htoi ("We", 2): 'W' ist keine Hexziffer und zaehlt als 0, 'e' ist 14.
 *
 *     type = 14.
 *
 * Das ist nicht ungefaehr der Fall aus L205, das IST der Wert aus L205: Dort kam
 * overlay[0].type als 14 statt 2 an und sah gueltig genug aus, um durch jede Pruefung zu
 * kommen. Gueltig ist er nicht -- src/overlay/overlay.h laesst 0..10 zu.
 *
 * Gewaehlte Loesung: EINE Laengenpruefung vorne statt 45 Einzelpruefungen. Jede Kommandoart
 * hat eine feste Mindestbreite; passt sie nicht, wird das Kommando GANZ verworfen statt halb
 * ausgefuehrt. Halb ausgefuehrt ist hier die schlechtere Haelfte: Ein fehlendes Kommando
 * laesst den alten Wert stehen und faellt beim naechsten Abgleich auf, ein halb ausgefuehrtes
 * schreibt einen falschen und faellt nirgends auf.
 *
 * Warum nicht 45 Einzelpruefungen: Das waeren 45 Gelegenheiten, eine zu vergessen, und die
 * naechste hinzugefuegte Kommandoart haette wieder keine. Diese Tabelle steht unmittelbar vor
 * dem switch, den sie beschreibt -- weicht eine Breite ab, faellt es beim Lesen auf.
 *
 * Rueckgabe: Mindestzahl Zeichen ab parameters[0] EINSCHLIESSLICH Kommandobuchstabe.
 *            0 = Kommandobuchstabe unbekannt. Dann wird nichts geprueft, weil der switch in
 *            var_set_parameter() die Zeile ohnehin wirkungslos verwirft.
 *
 * parameters[0] ist beim Aufruf garantiert != '\0' (der Aufrufer prueft es), parameters[1]
 * darf deshalb gelesen werden -- schlimmstenfalls ist es der Terminator, und der trifft
 * keinen case.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
var_cmd_min_len (const char * parameters)
{
    switch (parameters[0])
    {
        case CMD_CODE_NUMERIC_VAR:                  return  7;              // N ii ll hh
        case CMD_CODE_NUMERIC_ARRAY:                return  7;              // n ii nn bb
        case CMD_CODE_STRING_VAR:                   return  3;              // S ii + Text, Text darf leer sein
        case CMD_CODE_TIME_VAR:                     return 17;              // T ii YYYYMMDDhhmmss, Ziffern direkt indiziert
        case CMD_CODE_NIGHT_TIME_TABLE:             return  9;              // t ii mmmm ff
        case CMD_CODE_AMBILIGHT_NIGHT_TIME_TABLE:   return  9;              // a ii mmmm ff
        case CMD_CODE_ALARM_TIME_TABLE:             return  9;              // l ii mmmm ff
        case CMD_CODE_IR_CODE:                      return 13;              // I ii pp aaaa cccc

        case CMD_CODE_DISPLAY_VAR:                                          // D
            if (parameters[1] == PAR_CODE_DISPLAY_COLOR)                    // DC ii rr gg bb ww
            {
                /* Immer 12, NICHT von DISPLAY_USE_RGBW_NUM_VAR abhaengig. Der switch unten
                 * liest das ww zwar nur bei RGBW, der STM sendet es aber in beiden Bauarten:
                 * "DC%02x%02x%02x%02x%02x" mit White, sonst "DC%02x%02x%02x%02x00" mit einer
                 * literalen 00 (src/vars/vars.c:426/428, seit dem ersten Checkin unveraendert).
                 *
                 * Ein erster Entwurf machte die Breite hier von get_numvar() abhaengig. Das
                 * waere eine Pruefung gewesen, die von einem Wert abhaengt, den dieselbe
                 * Bruecke liefert -- geht das use_rgbw-Kommando verloren, verwirft der ESP
                 * danach gueltige Farbkommandos. Eine Konstante kann das nicht.
                 */
                return 12;
            }
            return 4;                                                       // unbekannter Unterbuchstabe: nur der Kopf

        case CMD_CODE_ANIMATION_VAR:                                        // A
        case CMD_CODE_COLOR_ANIMATION_VAR:                                  // C
        case CMD_CODE_AMBILIGHT_MODE_VAR:                                   // M
            switch (parameters[1])                                          // N/D/E/F -- in allen drei Gruppen dieselben
            {                                                               // Buchstaben, siehe die PAR_CODE_*-Defines oben
                case PAR_CODE_ANIMATION_MODE_NAME:              return 4;   // xN ii + Name, Name darf leer sein
                case PAR_CODE_ANIMATION_DECELERATION:           return 6;   // xD ii <wert:2>
                case PAR_CODE_ANIMATION_DEFAULT_DECELERATION:   return 6;   // xE ii <wert:2>
                case PAR_CODE_ANIMATION_FLAGS:                  return 6;   // xF ii <wert:2>
            }
            return 4;

        case CMD_CODE_OVERLAY_VAR:                                          // O
            switch (parameters[1])
            {
                case PAR_CODE_OVERLAY_TEXT:                     return 4;   // ON ii + Text, Text darf leer sein
                case PAR_CODE_OVERLAY_DATE_START:               return 8;   // OS ii <wert:4> -- als einziges vier Stellen
                case PAR_CODE_OVERLAY_TYPE:                     return 6;
                case PAR_CODE_OVERLAY_INTERVAL:                 return 6;
                case PAR_CODE_OVERLAY_DURATION:                 return 6;
                case PAR_CODE_OVERLAY_DATE_CODE:                return 6;
                case PAR_CODE_OVERLAY_DAYS:                     return 6;
                case PAR_CODE_OVERLAY_FLAGS:                    return 6;
            }
            return 4;
    }

    return 0;                                                               // unbekanntes Kommando
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * var_cmd_reject () - ein verworfenes Kommando festhalten, Befund L237
 *
 * Es darf nicht still verschwinden. Es darf aber auch nicht die Bruecke zusaetzlich belasten:
 * L109 beschreibt genau diese Mitkopplung -- eine Meldung ueber einen Uebertragungsfehler legt
 * Last auf dieselbe Leitung, deren Ueberlastung den Fehler erzeugt hat, und der RX-Ring des
 * STM ist 256 Byte gross und verwirft bei Ueberlauf still.
 *
 * Deshalb AUSSCHLIESSLICH stm32_log_append(): ein RAM-Ring im ESP, abrufbar ueber
 * /api/stm32_log, Kosten auf der UART null. Kein Serial.println, kein debugmsg.
 *
 * Gedrosselt, weil der Ring nur STM32_LOG_LINES Zeilen fasst und auch die Diagnosezeilen
 * traegt: die ersten vier Faelle einzeln, danach jeder fuenfzigste. Der laufende Zaehler steht
 * IN der Zeile -- die Gesamtzahl geht also nicht verloren, auch wenn nur die letzte uebrig ist.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static uint32_t     var_cmd_reject_cnt = 0;

static void
var_cmd_reject (const char * parameters, unsigned int have, unsigned int want)
{
    var_cmd_reject_cnt++;

    if (var_cmd_reject_cnt <= 4 || (var_cmd_reject_cnt % 50) == 0)
    {
        char line[80];

        snprintf (line, sizeof (line), "- var verworfen #%lu len=%u<%u: %.32s",
                  (unsigned long) var_cmd_reject_cnt, have, want, parameters);
        stm32_log_append (line);
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * var_set_parameter () - eine Kommandozeile der Bruecke auswerten
 *
 * Rueckgabe: 1 = Zeile war formal verwertbar, 0 = verworfen, weil zu kurz fuer ihre Kommandoart.
 *
 * Das heisst bewusst NICHT "Wert uebernommen": Eine formal gueltige Zeile mit unbekanntem
 * Kommandobuchstaben liefert ebenfalls 1, obwohl sie nichts bewirkt -- eine Wiederholung
 * wuerde daran nichts aendern. Dass die Quittung in ESP-uclock.ino nur den Empfang bestaetigt
 * und nicht die Uebernahme, bleibt der offene Punkt L233 und wird hier nicht geloest.
 *
 * Der Rueckgabewert ist der Haken aus A32 (specs/bruecke-wiederholung), und er WIRD seit
 * A32 ausgewertet: Der var-Zweig in ESP-uclock.ino antwortet bei 0 mit "!v" statt mit ".",
 * damit der STM die Zeile nachsendet -- das ist der Mechanismus, mit dem L205 geschlossen
 * wurde. Hier stand bis zum 05.10.2026 das Gegenteil ("Heute wertet ihn niemand aus"), und
 * das war keine verschobene Zeilennummer, sondern eine falsche Zusicherung im Quelltext
 * (L274): Wer nur den Kommentar liest, haelt den Rueckkanal fuer ungebaut.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
var_set_parameter (char * parameters)
{
    uint_fast8_t        cmd_code;
    uint_fast8_t        var_idx;
    uint_fast8_t        min_len;
    size_t              len;

#ifdef DEBUG
    debugmsg ("VAR", parameters);
#endif

    /* L237: EINE Pruefung vorne statt 45 einzelner, siehe var_cmd_min_len() darueber.
     * Ab hier gilt: Die Zeile ist lang genug fuer ihre Kommandoart. Jedes parameters += 2
     * im folgenden switch bleibt damit innerhalb der Zeile, und kein htoi liest hinter dem
     * Terminator.
     */
    len     = strlen (parameters);
    min_len = len ? var_cmd_min_len (parameters) : 1;                       // leere Zeile: immer verwerfen

    if (len < min_len)
    {
        var_cmd_reject (parameters, (unsigned int) len, (unsigned int) min_len);
        return 0;
    }

    cmd_code    = *parameters++;

    switch (cmd_code)
    {
        case CMD_CODE_NUMERIC_VAR:                                          // N: numeric variable: Niillhh
        {
            uint_fast8_t    lo;
            uint_fast8_t    hi;
            uint_fast16_t   val;

            var_idx = htoi (parameters, 2);
            parameters += 2;
            lo      = htoi (parameters, 2);
            parameters += 2;
            hi      = htoi (parameters, 2);
            parameters += 2;
            val     = (hi << 8) | lo;

            if (var_idx < MAX_NUM_VARIABLES)
            {
                numvars[var_idx] = val;
            }
            break;
        }

        case CMD_CODE_NUMERIC_ARRAY:                                        // A: numeric array: Niinnbb
        {
            uint_fast8_t    n;
            uint_fast8_t    val;

            var_idx = htoi (parameters, 2);
            parameters += 2;

            n = htoi (parameters, 2);
            parameters += 2;

            val = htoi (parameters, 2);

            switch (var_idx)
            {
                case DISPLAY_DIMMED_DISPLAY_COLORS:
                {
                    if (n < sizeof (dimmed_display_colors))
                    {
                        if (val <= MAX_BRIGHTNESS)
                        {
                            dimmed_display_colors[n] = val;
                        }
                    }
                    break;
                }
                case DISPLAY_DIMMED_AMBILIGHT_COLORS:
                {
                    if (n < sizeof (dimmed_ambilight_colors))
                    {
                        if (val <= MAX_BRIGHTNESS)
                        {
                            dimmed_ambilight_colors[n] = val;
                        }
                    }
                    break;
                }
            }
            break;
        }

        case CMD_CODE_STRING_VAR:                                           // S string variable: Siissssssss...
        {
            var_idx = htoi (parameters, 2);
            parameters += 2;

            if (var_idx < MAX_STR_VARIABLES)
            {
                memset (strvars[var_idx].str, 0, strvars[var_idx].maxlen + 1);
                strncpy (strvars[var_idx].str, parameters, strvars[var_idx].maxlen);
                strvars[var_idx].str[strvars[var_idx].maxlen] = '\0';
            }
            break;
        }

        case CMD_CODE_TIME_VAR:                                             // T time: TYYYYMMDDhhmmss
        {
            var_idx = htoi (parameters, 2);
            parameters += 2;

            if (var_idx < MAX_TM_VARIABLES)
            {
                tmvars[var_idx].tm_year = 1000 * (parameters[0]  - '0') +
                                           100 * (parameters[1]  - '0') +
                                            10 * (parameters[2]  - '0') +
                                             1 * (parameters[3]  - '0');
                tmvars[var_idx].tm_mon  =   10 * (parameters[4]  - '0') +
                                             1 * (parameters[5]  - '0');
                tmvars[var_idx].tm_mday =   10 * (parameters[6]  - '0') +
                                             1 * (parameters[7]  - '0');
                tmvars[var_idx].tm_hour =   10 * (parameters[8]  - '0') +
                                             1 * (parameters[9]  - '0');
                tmvars[var_idx].tm_min  =   10 * (parameters[10] - '0') +
                                             1 * (parameters[11] - '0');
                tmvars[var_idx].tm_sec =    10 * (parameters[12] - '0') +
                                             1 * (parameters[13] - '0');

                tmvars[var_idx].tm_wday = dayofweek (tmvars[var_idx].tm_mday, tmvars[var_idx].tm_mon + 1, tmvars[var_idx].tm_year + 1900);
            }

            break;
        }

        case CMD_CODE_DISPLAY_VAR:                                          // D: display
        {
            cmd_code = *parameters++;
            var_idx = htoi (parameters, 2);
            parameters += 2;

            switch (cmd_code)
            {
                case PAR_CODE_DISPLAY_COLOR:                                // DC: Display Color
                {
                    uint_fast8_t use_rgbw = get_numvar (DISPLAY_USE_RGBW_NUM_VAR);

                    if (var_idx < MAX_DSP_COLOR_VARIABLES)
                    {
                        dspcolorvars[var_idx].red = htoi (parameters, 2);
                        parameters += 2;
                        dspcolorvars[var_idx].green = htoi (parameters, 2);
                        parameters += 2;
                        dspcolorvars[var_idx].blue = htoi (parameters, 2);
                        parameters += 2;

                        if (use_rgbw)
                        {
                            dspcolorvars[var_idx].white = htoi (parameters, 2);
                            parameters += 2;
                        }
                    }
                    break;
                }
            }
            break;
        }

        case CMD_CODE_ANIMATION_VAR:                                        // A: animation
        {
            cmd_code = *parameters++;
            var_idx = htoi (parameters, 2);
            parameters += 2;

            if (var_idx < MAX_DISPLAY_ANIMATION_VARIABLES)
            {
                if (max_display_animation_variables < var_idx + 1)
                {
                    max_display_animation_variables = var_idx + 1;
                }
  
                switch (cmd_code)
                {
                    case PAR_CODE_ANIMATION_MODE_NAME:                          // AN: Animation mode Name
                    {
                        strncpy (displayanimationvars[var_idx].name, parameters, MAX_DISPLAY_ANIMATION_NAME_LEN);
                        break;
                    }
    
                    case PAR_CODE_ANIMATION_DECELERATION:                       // AD: Animation Deceleration
                    {
                        uint_fast8_t deceleration = htoi (parameters, 2);
                        parameters += 2;
                        displayanimationvars[var_idx].deceleration = deceleration;
                        break;
                    }
    
                    case PAR_CODE_ANIMATION_DEFAULT_DECELERATION:               // AE: Animation default deceleration
                    {
                        uint_fast8_t default_deceleration = htoi (parameters, 2);
                        parameters += 2;
                        displayanimationvars[var_idx].default_deceleration = default_deceleration;
                        break;
                    }
    
                    case PAR_CODE_ANIMATION_FLAGS:                              // AF: Animation Flags
                    {
                        uint_fast8_t flags = htoi (parameters, 2);
                        parameters += 2;
                        displayanimationvars[var_idx].flags = flags;
                        break;
                    }
                }
            }
            break;
        }

        case CMD_CODE_COLOR_ANIMATION_VAR:                                  // C: Color animation
        {
            cmd_code = *parameters++;
            var_idx = htoi (parameters, 2);
            parameters += 2;

            switch (cmd_code)
            {
                case PAR_CODE_COLOR_ANIMATION_MODE_NAME:                    // CN: Color animation Name
                {
                    if (var_idx < MAX_COLOR_ANIMATION_VARIABLES)
                    {
                        strncpy (coloranimationvars[var_idx].name, parameters, MAX_COLOR_ANIMATION_NAME_LEN);
                    }
                    break;
                }

                case PAR_CODE_COLOR_ANIMATION_DECELERATION:                 // CD: Color animation Deceleration
                {
                    if (var_idx < MAX_COLOR_ANIMATION_VARIABLES)
                    {
                        uint_fast8_t deceleration = htoi (parameters, 2);
                        parameters += 2;
                        coloranimationvars[var_idx].deceleration = deceleration;
                    }
                    break;
                }

                case PAR_CODE_COLOR_ANIMATION_DEFAULT_DECELERATION:         // CE: Color animation default deceleration
                {
                    if (var_idx < MAX_COLOR_ANIMATION_VARIABLES)
                    {
                        uint_fast8_t default_deceleration = htoi (parameters, 2);
                        parameters += 2;
                        coloranimationvars[var_idx].default_deceleration = default_deceleration;
                    }
                    break;
                }

                case PAR_CODE_COLOR_ANIMATION_FLAGS:                        // CF: Animation Flags
                {
                    if (var_idx < MAX_COLOR_ANIMATION_VARIABLES)
                    {
                        uint_fast8_t flags = htoi (parameters, 2);
                        parameters += 2;
                        coloranimationvars[var_idx].flags = flags;
                    }
                    break;
                }
            }
            break;
        }

        case CMD_CODE_AMBILIGHT_MODE_VAR:                                   // Ambilight mode
        {
            cmd_code = *parameters++;
            var_idx = htoi (parameters, 2);
            parameters += 2;

            switch (cmd_code)
            {
                case PAR_CODE_AMBILIGHT_MODE_NAME:                          // MN: Ambilight mode Name
                {
                    if (var_idx < MAX_AMBILIGHT_MODE_VARIABLES)
                    {
                        strncpy (ambilightmodevars[var_idx].name, parameters, MAX_AMBILIGHT_MODE_NAME_LEN);
                    }
                    break;
                }

                case PAR_CODE_AMBILIGHT_MODE_DECELERATION:                  // MD: Ambilight mode Deceleration
                {
                    if (var_idx < MAX_AMBILIGHT_MODE_VARIABLES)
                    {
                        uint_fast8_t deceleration = htoi (parameters, 2);
                        parameters += 2;
                        ambilightmodevars[var_idx].deceleration = deceleration;
                    }
                    break;
                }

                case PAR_CODE_AMBILIGHT_MODE_DEFAULT_DECELERATION:          // ME: Ambilight mode default deceleration
                {
                    if (var_idx < MAX_AMBILIGHT_MODE_VARIABLES)
                    {
                        uint_fast8_t default_deceleration = htoi (parameters, 2);
                        parameters += 2;
                        ambilightmodevars[var_idx].default_deceleration = default_deceleration;
                    }
                    break;
                }

                case PAR_CODE_AMBILIGHT_MODE_FLAGS:                         // MF: Ambilight mode Flags
                {
                    if (var_idx < MAX_AMBILIGHT_MODE_VARIABLES)
                    {
                        uint_fast8_t flags = htoi (parameters, 2);
                        parameters += 2;
                        ambilightmodevars[var_idx].flags = flags;
                    }
                    break;
                }
            }
            break;
        }

        case CMD_CODE_OVERLAY_VAR:                                          // O: overlay
        {
            cmd_code = *parameters++;
            var_idx = htoi (parameters, 2);
            parameters += 2;

            switch (cmd_code)
            {
                case PAR_CODE_OVERLAY_TYPE:                                 // OT: overlay type
                    if (var_idx < MAX_OVERLAYS)
                    {
                        uint_fast8_t type = htoi (parameters, 2);
                        parameters += 2;
                        overlays[var_idx].type = type;
                    }
                    break;
                case PAR_CODE_OVERLAY_INTERVAL:                             // OI: overlay interval
                    if (var_idx < MAX_OVERLAYS)
                    {
                        uint_fast8_t interval = htoi (parameters, 2);
                        parameters += 2;
                        overlays[var_idx].interval = interval;
                    }
                    break;
                case PAR_CODE_OVERLAY_DURATION:                             // OD: overlay duration
                    if (var_idx < MAX_OVERLAYS)
                    {
                        uint_fast8_t duration = htoi (parameters, 2);
                        parameters += 2;
                        overlays[var_idx].duration = duration;
                    }
                    break;
                case PAR_CODE_OVERLAY_DATE_CODE:                            // OC: overlay date code
                    if (var_idx < MAX_OVERLAYS)
                    {
                        uint_fast8_t date_code = htoi (parameters, 2);
                        parameters += 2;
                        overlays[var_idx].date_code = date_code;
                    }
                    break;
                case PAR_CODE_OVERLAY_DATE_START:                           // OS: overlay date start
                    if (var_idx < MAX_OVERLAYS)
                    {
                        uint_fast16_t date_start = htoi (parameters, 4);
                        parameters += 4;
                        overlays[var_idx].date_start = date_start;
                    }
                    break;
                case PAR_CODE_OVERLAY_DAYS:                                 // OY: overlay date end
                    if (var_idx < MAX_OVERLAYS)
                    {
                        uint_fast16_t days = htoi (parameters, 2);
                        parameters += 2;
                        overlays[var_idx].days = days;
                    }
                    break;
                case PAR_CODE_OVERLAY_TEXT:                                 // ON: overlay name or text
                    if (var_idx < MAX_OVERLAYS)
                    {
                        strncpy (overlays[var_idx].text, parameters, OVERLAY_MAX_TEXT_LEN);
                    }
                    break;
                case PAR_CODE_OVERLAY_FLAGS:                                // OF: overlay flags
                    if (var_idx < MAX_OVERLAYS)
                    {
                        uint_fast8_t flags = htoi (parameters, 2);
                        parameters += 2;
                        overlays[var_idx].flags = flags;
                    }
                    break;
            }
            break;
        }

        case CMD_CODE_NIGHT_TIME_TABLE:                                     // tiimm night table: minutes + flags
        case CMD_CODE_AMBILIGHT_NIGHT_TIME_TABLE:                           // aiimm ambilight night table: minutes + flags
        {
            uint_fast16_t minutes;
            uint_fast8_t flags;

            var_idx = htoi (parameters, 2);
            parameters += 2;
            minutes = htoi (parameters, 2) + (htoi (parameters + 2, 2) << 8);
            parameters += 4;
            flags = htoi (parameters, 2);
            parameters += 2;

            if (var_idx < MAX_NIGHT_TIME_VARIABLES)
            {
                if (cmd_code == 't')
                {
                    nighttimevars[var_idx].minutes = minutes;
                    nighttimevars[var_idx].flags = flags;
                }
                else
                {
                    ambilightnighttimevars[var_idx].minutes = minutes;
                    ambilightnighttimevars[var_idx].flags = flags;
                }
            }

            break;
        }

        case CMD_CODE_ALARM_TIME_TABLE:                                     // liimm alarm table: minutes + flags
        {
            uint_fast16_t minutes;
            uint_fast8_t flags;

            var_idx = htoi (parameters, 2);
            parameters += 2;
            minutes = htoi (parameters, 2) + (htoi (parameters + 2, 2) << 8);
            parameters += 4;
            flags = htoi (parameters, 2);
            parameters += 2;

            if (var_idx < MAX_ALARM_TIME_VARIABLES)
            {
                alarmtimevars[var_idx].minutes = minutes;
                alarmtimevars[var_idx].flags = flags;
            }

            break;
        }

        case CMD_CODE_IR_CODE:                                              // I<idx:2><protocol:2><address:4><command:4>
        {
            uint_fast8_t    protocol;
            uint_fast16_t   address;
            uint_fast16_t   command;

            var_idx = htoi (parameters, 2);                                 // feste Breiten, siehe var_send_ir_code() im STM
            parameters += 2;
            protocol = htoi (parameters, 2);
            parameters += 2;
            address = htoi (parameters, 4);
            parameters += 4;
            command = htoi (parameters, 4);
            parameters += 4;

            if (var_idx < MAX_IR_CODES)
            {
                ir_codes[var_idx].protocol  = protocol;
                ir_codes[var_idx].address   = address;
                ir_codes[var_idx].command   = command;
                ir_codes_received_mask     |= (((uint32_t) 1) << var_idx);
            }
            /* Ein Index jenseits von MAX_IR_CODES wird verworfen -- aber nicht spurlos: Sein Bit
             * fehlt in der Maske, ir_codes_is_complete() bleibt 0 und der Endpunkt nennt ihn in
             * missing[]. Der Abzug scheitert damit sichtbar statt halb zu gelingen.
             */

            break;
        }
    }

    return 1;
}

void
vars_init (void)
{
    numvars[HARDWARE_CONFIGURATION_NUM_VAR] = 0xFFFF;
    numvars[AMBILIGHT_IS_UP_NUM_VAR] = 1;

    /* strvar 3 war in jedem Abzug leer: Der STM32 fuellt ihn nicht (var_send_esp8266_version
     * ist leer) und weist ein Setzen als readonly zurueck - niemand war zustaendig (L55).
     * Die eigene Version kennt nur der ESP, also fuellt er den Platz selbst. Bewusst ohne
     * set_strvar, damit beim Start kein zusaetzliches CMD auf die STM-UART geht, das dort
     * ohnehin nur eine readonly-Meldung ausloest.
     */
    utf8_copy_truncated (strvars[ESP8266_VERSION_STR_VAR].str, ESP_VERSION, strvars[ESP8266_VERSION_STR_VAR].maxlen);
}
