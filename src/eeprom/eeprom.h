/*-------------------------------------------------------------------------------------------------------------------------------------------
 * eeprom.h - delarations of EEPROM routines
 *
 * Copyright (c) 2014-2026 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifndef EEPROM_H
#define EEPROM_H

#if defined (STM32F10X)
#include "stm32f10x.h"
#elif defined (STM32F4XX)
#include "stm32f4xx.h"
#endif

extern uint_fast8_t             eeprom_is_up;
extern volatile uint_fast8_t    eeprom_ms_tick;

extern uint_fast8_t             eeprom_init (uint32_t);
extern uint_fast8_t             eeprom_get_address (void);
extern uint_fast8_t             eeprom_read (uint_fast16_t, uint8_t *, uint_fast16_t);
extern uint_fast8_t             eeprom_write (uint_fast16_t, uint8_t *, uint_fast16_t);

/* Zeitbasis der Messzeile in eeprom_write(), definiert in main.c (Paket 2026-10-09, M.1). Hier und
 * nicht in main.h: main.c bindet diesen Header ueber eep.h ein, der Compiler gleicht die beiden
 * Deklarationen also gegen die Definition ab -- und main.h ist eine Versionsdatei (R4).
 */
extern uint32_t                 diag_ticks (void);
extern const uint32_t           diag_ticks_per_ms;

#endif
