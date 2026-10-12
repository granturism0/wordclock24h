/*-------------------------------------------------------------------------------------------------------------------------------------------
 * log.c - log routines
 *
 * Copyright (c) 2016-2026 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include "log.h"
#include "esp8266.h"

#undef UART_PREFIX
#define UART_PREFIX     log
#include "uart.h"

#define STRBUF_SIZE 256

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * log a formatted message with a fixed prefix (by va_list)
 *
 * N6 (S3.2): Praefix und Text gehen in DENSELBEN Puffer, damit je Aufruf weiter genau EINE Zeile an die
 * UART und an den ESP-Ring geht (esp8266_send_log_line). Ein Praefix im Formattext haette dieselbe Ausgabe
 * ergeben, stand aber 48-mal im Flash (dfplayer.c). Abschneiden wie bisher: vsnprintf bekommt den Rest des
 * Puffers, zusammen also hoechstens STRBUF_SIZE - 1 Zeichen -- dieselben wie mit dem Praefix im Format.
 * Der Praefix ist ein kurzes festes Literal (weit unter STRBUF_SIZE).
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
log_vprintf_prefix (const char * prefix, const char * fmt, va_list ap)
{
    static char str_buf[STRBUF_SIZE];
    char *      p = str_buf;

    while (*prefix)
    {
        *p++ = *prefix++;
    }

    (void) vsnprintf (p, STRBUF_SIZE - (p - str_buf), fmt, ap);
    (void) log_uart_puts (str_buf);
    esp8266_send_log_line (str_buf);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * log a formatted message (by va_list)
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
log_vprintf (const char * fmt, va_list ap)
{
    log_vprintf_prefix ("", fmt, ap);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * log a formatted message with a fixed prefix (varargs), N6
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
log_printf_prefix (const char * prefix, const char * fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    log_vprintf_prefix (prefix, fmt, ap);
    va_end (ap);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * log a formatted message (varargs)
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
log_printf (const char * fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    log_vprintf (fmt, ap);
    va_end (ap);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * init log functions
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
log_init (uint32_t baudrate)
{
    log_uart_init (baudrate);
}
