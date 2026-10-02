/*-----------------------------------------------------------------------------------------------------------------------------------------------
 * tempsensor.c - temperature sensor routines
 *
 * Copyright (c) 2015-2026 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-----------------------------------------------------------------------------------------------------------------------------------------------
 */

#include "ds18xx.h"
#include "tempsensor.h"
#include "eep.h"
#include "eeprom-data.h"

TEMP_GLOBALS    gtemp =
{
    0,                                                                      // correction
    0xFF                                                                    // index
};

/*-----------------------------------------------------------------------------------------------------------------------------------------------
 * temp_start_conversion () - start conversion
 *-----------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
temp_start_conversion (uint_fast8_t do_wait)
{
    uint_fast8_t    rtc;

    rtc = ds18xx_start_conversion (do_wait);

    return rtc;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------
 * temp_read_temp_index () - read temperature
 *
 *    temperature_index =   0 ->   0°C
 *    temperature_index = 250 -> 125°C
 *    temperature_index = 255 -> Error
 *-----------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
temp_read_temp_index (void)
{
    uint_fast8_t    resolution;
    uint_fast8_t    is_negative;
    uint_fast16_t   raw_temp;
    uint_fast8_t    index = 0xFF;
    int_fast16_t    corrected;

    if (ds18xx_read_raw_temp (&resolution, &is_negative, &raw_temp))
    {
        if (! is_negative)
        {
            corrected = (int_fast16_t) raw_temp;
            corrected -= gtemp.correction;                          // correct temperature due to self-heating

            // Der Index kennt nur 0..250 (0..125 Grad), 255 ist der Fehlerwert. Begrenzen,
            // damit eine Korrektur ueber den Rand hinaus nicht umlaeuft und als Fehlerwert
            // oder als absurder Messwert beim Nutzer landet.
            if (corrected < 0)
            {
                corrected = 0;
            }
            else if (corrected > 250)
            {
                corrected = 250;
            }

            index = (uint_fast8_t) corrected;
        }
        gtemp.index = index;
    }
    return index;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read configuration from EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
temp_read_config_from_eep (uint32_t eep_version)
{
    uint_fast8_t            rtc = 0;
    uint8_t                 temp_correction8;
    int_fast8_t             corr;

    if (eep_is_up)
    {
        if (eep_version >= EEPROM_VERSION_2_1)
        {
            rtc = eep_read (EEPROM_DATA_OFFSET_DS18XX_TEMP_CORR, &temp_correction8, EEPROM_DATA_SIZE_DS18XX_TEMP_CORR);

            corr = (int8_t) temp_correction8;                        // Zweierkomplement, siehe tempsensor.h

            if (corr < -TEMP_CORRECTION_LIMIT || corr > TEMP_CORRECTION_LIMIT)
            {
                corr = 0;
            }
        }
        else
        {
            corr = 0;
            rtc = 1;
        }

        gtemp.correction = corr;
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write configuration to EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
temp_write_config_to_eep (void)
{
    uint_fast8_t            rtc = 0;
    uint8_t                 temp_correction8;

    temp_correction8    = (uint8_t) gtemp.correction;               // negativ wird zum Zweierkomplement-Byte, Layout bleibt 1 Byte

    if (eep_is_up)
    {
        if (eep_write (EEPROM_DATA_OFFSET_DS18XX_TEMP_CORR, &temp_correction8, EEPROM_DATA_SIZE_DS18XX_TEMP_CORR))
        {
            rtc = 1;
        }
    }

    return rtc;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------
 * get temperature correction
 *-----------------------------------------------------------------------------------------------------------------------------------------------
 */
int_fast8_t
temp_get_temp_correction (void)
{
    return gtemp.correction;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------
 * set temperature correction
 *-----------------------------------------------------------------------------------------------------------------------------------------------
 */
int_fast8_t
temp_set_temp_correction (int_fast8_t new_temp_correction)
{
    // Begrenzen statt uebernehmen: ein verstuemmeltes UART-Kommando soll keinen
    // Unsinn ins EEPROM schreiben, aus dem er nach jedem Neustart wieder hervorkommt.
    if (new_temp_correction < -TEMP_CORRECTION_LIMIT)
    {
        new_temp_correction = -TEMP_CORRECTION_LIMIT;
    }
    else if (new_temp_correction > TEMP_CORRECTION_LIMIT)
    {
        new_temp_correction = TEMP_CORRECTION_LIMIT;
    }

    gtemp.correction = new_temp_correction;
    temp_write_config_to_eep ();
    return gtemp.correction;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------
 * temp_init () - initialize temperature sensor routines
 *-----------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
temp_init (void)
{
    uint_fast8_t    rtc;

    rtc = ds18xx_init (DS_RESOLUTION_9_BIT);

    return rtc;
}

