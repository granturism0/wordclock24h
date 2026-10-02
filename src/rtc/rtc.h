/*-------------------------------------------------------------------------------------------------------------------------------------------
 * rtc.h - delarations of DS3231 RTC routines
 *
 * Copyright (c) 2014-2026 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifndef RTC_H
#define RTC_H

#include <time.h>
#if defined (STM32F10X)
#  include "stm32f10x.h"
#elif defined (STM32F4XX)
#  include "stm32f4xx.h"
#endif

typedef struct
{
    uint_fast8_t                rtc_is_up;
    int_fast8_t                 rtc_temp_correction;
    uint_fast8_t                rtc_temperature_index;
} RTC_GLOBALS;

extern RTC_GLOBALS              grtc;

// Die Korrektur kommt vom ESP als Zweierkomplement-Byte ueber die UART und liegt so auch
// im EEPROM. Vorzeichenlos gelesen wurde aus -1 eine 255 und daraus ein Messwert von zig
// tausend Grad. Das Drahtformat bleibt wie es ist, nur die Auswertung ist vorzeichenbehaftet.
#define RTC_TEMP_CORRECTION_LIMIT   20                      // Grenze der Oberflaeche: -20..20

extern uint_fast8_t rtc_init (uint32_t);
extern uint_fast8_t rtc_set_date_time (struct tm *);
extern uint_fast8_t rtc_get_date_time (struct tm *);
extern uint_fast8_t rtc_read_config_from_eep (uint32_t);
extern uint_fast8_t rtc_write_config_to_eep (void);
extern int_fast8_t  rtc_get_temp_correction (void);
extern int_fast8_t  rtc_set_temp_correction (int_fast8_t);
extern uint_fast8_t rtc_get_temperature_index (void);

#endif
