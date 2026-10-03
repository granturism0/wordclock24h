/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * esp8266-uart.c - definitions of uart driver routines
 *
 * Copyright (c) 2015-2026 Frank Meyer - frank(at)uclock.de
 *
 * Possible UARTs of STM32F10x:
 *           ALTERNATE=0    ALTERNATE=1    ALTERNATE=2
 *  +--------------------------------------------------+
 *  | UART | TX   | RX   || TX   | RX   || TX   | RX   |
 *  |======|======|======||======|======||======|======|
 *  | 1    | PA9  | PA10 || PB6  | PB7  ||      |      |
 *  | 2    | PA2  | PA3  || PD5  | PD6  ||      |      |
 *  | 3    | PB10 | PB11 || PC10 | PC11 || PD8  | PD9  |
 *  +--------------------------------------------------+
 *
 * Possible UARTs of STM32F4xx Nucleo:
 *           ALTERNATE=0    ALTERNATE=1    ALTERNATE=2
 *  +--------------------------------------------------+
 *  | UART | TX   | RX   || TX   | RX   || TX   | RX   |
 *  |======|======|======||======|======||======|======|
 *  | 1    | PA9  | PA10 || PB6  | PB7  ||      |      |
 *  | 2    | PA2  | PA3  || PD5  | PD6  ||      |      |
 *  | 6    | PC6  | PC7  || PA11 | PA12 ||      |      |
 *  +--------------------------------------------------+
 *
 * Possible UARTs of STM32F407:
 *           ALTERNATE=0    ALTERNATE=1    ALTERNATE=2
 *  +--------------------------------------------------+
 *  | UART | TX   | RX   || TX   | RX   || TX   | RX   |
 *  |======|======|======||======|======||======|======|
 *  | 1    | PA9  | PA10 || PB6  | PB7  ||      |      |
 *  | 2    | PA2  | PA3  || PD5  | PD6  ||      |      |
 *  | 3    | PB10 | PB11 || PC10 | PC11 || PD8  | PD9  |
 *  | 4    | PA0  | PA1  || PC10 | PC11 ||      |      |
 *  | 5    | PC12 | PD2  ||      |      ||      |      |
 *  | 6    | PC6  | PC7  || PG14 | PG9  ||      |      |
 *  +--------------------------------------------------+
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#define UART_PREFIX             esp8266                 // see also esp8266.c

#if defined (BLACK_BOARD)                               // STM32F407VE Black Board: we use USART3 ALT0: PB10 | PB11
#  define UART_NUMBER           3                       // UART number on STM32F407VE (1-6 for UART)
#  define UART_ALTERNATE        0                       // ALTERNATE number

#elif defined (NUCLEO_BOARD)                            // STM32F4xx Nucleo Board: we use USART6 ALT1: PA11 | PA12
#  define UART_NUMBER           6                       // UART number on STM32F4xx (1-6 for UART)
#  define UART_ALTERNATE        1                       // ALTERNATE number

#elif defined (BLACKPILL_BOARD)                         // STM32F4xx BlackPill Board: we use USART6 ALT1: PA11 | PA12
#  define UART_NUMBER           2                       // UART number on STM32F4xx (1-6 for UART)
#  define UART_ALTERNATE        0                       // ALTERNATE number

#elif defined (BLUEPILL_BOARD)                          // STM32F103C8T6 BluePill Board: we use USART2 ALT0: PA2 | PA3
#  define UART_NUMBER           2                       // UART number on STM32F1xx
#  define UART_ALTERNATE        0                       // ALTERNATE number

#else
#  error unknown STM32
#endif

#define UART_TXBUFLEN           128                     // ringbuffer size for UART TX

/* 1024 statt 256 seit dem 03.10.2026.
 *
 * 256 Byte sind bei 115200 Baud nach 22,2 ms voll. So lange darf der Hauptloop nicht
 * blockieren, und er tut es regelmaessig -- ein einziger EEPROM-Schreibzugriff kostet rund
 * 16 ms (BEFUNDE.md, L144: Einbruch der Loopdurchlaeufe auf 58 % ueber rund 4,25 s, dabei
 * 702 verworfene Zeichen). Was ueberlaeuft, verwirft die ISR still.
 *
 * 1024 Byte decken rund 89 ms Blockade. Das ist keine Loesung der Blockaden -- die werden
 * getrennt angegangen --, sondern der Puffer dafuer, dass eine davon doch einmal laenger
 * dauert. Kosten: 768 Byte RAM, frei sind rund 9'660 B auf dem F103 und rund 117 kB auf dem
 * F411.
 *
 * ACHTUNG: Ueber 256 hinaus muessen die Ringindizes in uart-driver.h breiter als 8 Bit sein.
 * Bei genau 256 faellt der Ueberlauf eines 8-Bit-Index mit dem Ruecksetzen auf 0 zusammen --
 * darauf darf sich eine groessere Puffergroesse nicht verlassen. Siehe RINGINDIZES dort.
 */
#define UART_RXBUFLEN           1024                    // ringbuffer size for UART RX

#include "uart-driver.h"
