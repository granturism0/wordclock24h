/*-------------------------------------------------------------------------------------------------------------------------------------------
 * remote-ir.c - remote IR routines using IRMP
 *
 * Copyright (c) 2014-2026 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#include "wclock24h-config.h"
#include "irmp.h"
#include "remote-ir.h"
#include "display.h"
#include "eep.h"
#include "eeprom-data.h"
#include "log.h"
#include "delay.h"
#include "main.h"

static  IRMP_DATA   irmp_data_array[N_REMOTE_IR_CMDS];

/* Wartezeit je Taste. var_send_buf() bricht nach 3 s ab, dort wartet aber eine Maschine.
 * Hier wartet ein Mensch, der erst den Ticker lesen und dann die Fernbedienung suchen muss --
 * 30 s sind dafuer reichlich und lassen die Uhr trotzdem nicht ewig stehen, wenn niemand drueckt.
 */
#define REMOTE_IR_LEARN_TIMEOUT_MSEC    30000
#define REMOTE_IR_LEARN_POLL_MSEC       1

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * search for a previously stored IR command
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
search_cmd (IRMP_DATA * ip, uint_fast8_t max_cmds)
{
    uint_fast8_t    rtc = REMOTE_IR_CMD_INVALID;
    uint_fast8_t    i;

    for (i = 0; i < max_cmds; i++)
    {
        if (ip->protocol == irmp_data_array[i].protocol &&
            ip->address  == irmp_data_array[i].address  &&
            ip->command  == irmp_data_array[i].command)
        {
            rtc = i;
            break;
        }
    }
    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read an IR command
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
remote_ir_get_cmd (void)
{
    static uint_fast16_t    frame;
    static uint_fast8_t     last_repetition_flag;
    IRMP_DATA               irmp_data;
    uint_fast8_t            rtc = REMOTE_IR_CMD_INVALID;

    if (irmp_get_data (&irmp_data))
    {
        frame++;

        if (last_repetition_flag == 0 && (irmp_data.flags & IRMP_FLAG_REPETITION))  // ignore first repetition frame
        {
            last_repetition_flag = 1;
            return rtc;
        }

        last_repetition_flag = (irmp_data.flags & IRMP_FLAG_REPETITION) ? 1: 0;
        rtc = search_cmd (&irmp_data, N_REMOTE_IR_CMDS);
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * learn remote IR control
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
remote_ir_learn (void)
{
    uint_fast8_t    rtc = 1;
    uint_fast8_t    i;
    uint_fast32_t   waited_msec;
    IRMP_DATA       dummy;
    const char * t;

    for (i = 0; i < N_REMOTE_IR_CMDS && rtc; i++)                                   // rtc == 0: abgebrochen, Rest nicht mehr abfragen
    {
        switch (i)
        {
            case REMOTE_IR_CMD_POWER:                         t = "power off/on";                     break;
            case REMOTE_IR_CMD_OK:                            t = "ok";                               break;
            case REMOTE_IR_CMD_DECREMENT_DISPLAY_MODE:        t = "decrement display mode";           break;
            case REMOTE_IR_CMD_INCREMENT_DISPLAY_MODE:        t = "increment display mode";           break;
            case REMOTE_IR_CMD_DECREMENT_ANIMATION_MODE:      t = "decrement animation mode";         break;
            case REMOTE_IR_CMD_INCREMENT_ANIMATION_MODE:      t = "increment animation mode";         break;
            case REMOTE_IR_CMD_DECREMENT_HOUR:                t = "decrement hour";                   break;
            case REMOTE_IR_CMD_INCREMENT_HOUR:                t = "increment hour";                   break;
            case REMOTE_IR_CMD_DECREMENT_MINUTE:              t = "decrement minute";                 break;
            case REMOTE_IR_CMD_INCREMENT_MINUTE:              t = "increment minute";                 break;
            case REMOTE_IR_CMD_DECREMENT_BRIGHTNESS_RED:      t = "decrement red brightness";         break;
            case REMOTE_IR_CMD_INCREMENT_BRIGHTNESS_RED:      t = "increment red brightness";         break;
            case REMOTE_IR_CMD_DECREMENT_BRIGHTNESS_GREEN:    t = "decrement green brightness";       break;
            case REMOTE_IR_CMD_INCREMENT_BRIGHTNESS_GREEN:    t = "increment green brightness";       break;
            case REMOTE_IR_CMD_DECREMENT_BRIGHTNESS_BLUE:     t = "decrement blue brightness";        break;
            case REMOTE_IR_CMD_INCREMENT_BRIGHTNESS_BLUE:     t = "increment blue brightness";        break;
            case REMOTE_IR_CMD_DECREMENT_BRIGHTNESS:          t = "decrement global brightness";      break;
            case REMOTE_IR_CMD_INCREMENT_BRIGHTNESS:          t = "increment global brightness";      break;
            case REMOTE_IR_CMD_AUTO_BRIGHTNESS_CONTROL:       t = "toggle auto brightness";           break;
            case REMOTE_IR_CMD_GET_TEMPERATURE:               t = "get temperature";                  break;
        }

        display_set_ticker ((const unsigned char *) t, 1);
        log_message (t);

        irmp_get_data (&dummy);

        waited_msec = 0;

        while (1)
        {
            if (irmp_get_data (&irmp_data_array[i]))                                            // read ir data
            {
                if ((irmp_data_array[i].flags & IRMP_FLAG_REPETITION) == 0)                     // no repetition
                {                                                                               // accept if ...
                    if (i == 0 ||                                                               // ... it's the first key (power)
                        search_cmd (&irmp_data_array[i], i) == REMOTE_IR_CMD_INVALID ||         // ... key has not been stored previously
                        search_cmd (&irmp_data_array[i], i) == REMOTE_IR_CMD_POWER)             // ... user wants to skip it (by power key)
                    {
                        break;                                                                  // next command...
                    }
                }
            }

            /* Hier wird auf einen Menschen gewartet, ueber bis zu N_REMOTE_IR_CMDS Durchlaeufe.
             * Ohne Reload faellt der IWDG nach 20 s zu, obwohl das Warten gewollt ist -- deshalb
             * wird er bedient, wie in display_test(). Und weil "gewollt" nicht "unbegrenzt" heisst,
             * bricht die Schleife ab, wenn niemand eine Taste drueckt.
             */
            delay_msec (REMOTE_IR_LEARN_POLL_MSEC);
            watchdog_reload ();

            waited_msec += REMOTE_IR_LEARN_POLL_MSEC;

            if (waited_msec >= REMOTE_IR_LEARN_TIMEOUT_MSEC)
            {
                rtc = 0;                                                            // Aufrufer schreibt dann nichts ins EEPROM
                break;
            }
        }
    }

    if (rtc)
    {
        display_set_ticker ((const unsigned char *) "  Thank you!", 1);
    }
    else
    {
        /* Abgebrochen: irmp_data_array ist bis zur aktuellen Taste halb beschrieben, das EEPROM
         * dagegen unveraendert. Beides wieder in Deckung bringen, sonst reagiert die Uhr bis zum
         * naechsten Reset auf halb gelernte Codes.
         */
        remote_ir_read_codes_from_eep ();

        display_set_ticker ((const unsigned char *) "  timeout", 1);
        log_message ("IR learn aborted: timeout, codes unchanged");
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read IR codes from EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
remote_ir_read_codes_from_eep (void)
{
    uint8_t         packed_irmp_data[PACKED_IRMP_DATA_SIZE];
    uint_fast8_t    rtc = 0;

    if (eep_is_up)
    {
        uint_fast8_t    i;
        uint_fast16_t   start_addr = EEPROM_DATA_OFFSET_IRMP_DATA;

        for (i = 0; i < N_REMOTE_IR_CMDS; i++)
        {
            rtc = eep_read (start_addr, (uint8_t *) &packed_irmp_data, PACKED_IRMP_DATA_SIZE);

            irmp_data_array[i].protocol = packed_irmp_data[0];
            irmp_data_array[i].address  = packed_irmp_data[1] | (packed_irmp_data[2] << 8);
            irmp_data_array[i].command  = packed_irmp_data[3] | (packed_irmp_data[4] << 8);

            start_addr += PACKED_IRMP_DATA_SIZE;
        }
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write IR codes to EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
remote_ir_write_codes_to_eep (void)
{
    uint8_t         packed_irmp_data[PACKED_IRMP_DATA_SIZE];
    uint_fast8_t    rtc = 0;

    if (eep_is_up)
    {
        uint_fast8_t    i;
        uint_fast16_t   start_addr = EEPROM_DATA_OFFSET_IRMP_DATA;

        for (i = 0; i < N_REMOTE_IR_CMDS; i++)
        {
            packed_irmp_data[0]         = irmp_data_array[i].protocol;
            packed_irmp_data[1]         = irmp_data_array[i].address & 0xFF;
            packed_irmp_data[2]         = irmp_data_array[i].address >> 8;
            packed_irmp_data[3]         = irmp_data_array[i].command & 0xFF;
            packed_irmp_data[4]         = irmp_data_array[i].command >> 8;

            rtc = eep_write (start_addr, (uint8_t *) &packed_irmp_data, PACKED_IRMP_DATA_SIZE);
            start_addr += PACKED_IRMP_DATA_SIZE;
        }
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read a single IR code out of the RAM mirror
 *
 * irmp_data_array ist static, von aussen also nicht erreichbar. Diese Funktion ist der
 * einzige Lesezugang. Sie liefert den RAM-Spiegel und nicht das EEPROM: beide sind nach
 * jedem remote_ir_set_code() und nach remote_ir_read_codes_from_eep() in Deckung, und ein
 * EEPROM-Zugriff je Taste waere hier nur teurer.
 *
 * flags wird bewusst nicht geliefert. Das Feld ist eine Laufzeiteigenschaft des Empfangs
 * (IRMP_FLAG_REPETITION) und steht auch im EEPROM nicht.
 *
 * Rueckgabe: 1 = Werte geschrieben, 0 = nichts angefasst.
 *
 * Abgewiesen werden ein Index ausserhalb 0 .. N_REMOTE_IR_CMDS-1 und ein Nullzeiger. Kein
 * Zurechtbiegen auf Taste 0 -- das lieferte stillschweigend den Code einer fremden Taste.
 * Geprueft wird vor dem ersten Schreiben, damit bei Rueckgabe 0 kein Zeiger halb gefuellt ist.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
remote_ir_get_code (uint_fast8_t idx, uint_fast8_t * protocol, uint_fast16_t * address, uint_fast16_t * command)
{
    uint_fast8_t    rtc = 0;

    if (idx < N_REMOTE_IR_CMDS && protocol != (uint_fast8_t *) 0 && address != (uint_fast16_t *) 0 && command != (uint_fast16_t *) 0)
    {
        *protocol   = irmp_data_array[idx].protocol;
        *address    = irmp_data_array[idx].address;
        *command    = irmp_data_array[idx].command;

        rtc = 1;
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set a single IR code: RAM mirror and EEPROM together
 *
 * Geschrieben werden genau die 5 Byte dieser Taste an
 * EEPROM_DATA_OFFSET_IRMP_DATA + idx * PACKED_IRMP_DATA_SIZE, nicht alle 20. Ein Byte kostet
 * rund 16 ms, 5 Byte sind rund 80 ms Hauptloop-Blockade. Alle 20 Tasten am Stueck waeren rund
 * 1,6 s -- so lange liefe schedule_esp8266_messages() nicht, und der 256-Byte-Empfangsring der
 * ESP-Bruecke verwirft bei Ueberlauf still, ohne Log und ohne Fehler.
 *
 * Packung wie in remote_ir_write_codes_to_eep(): [0] protocol, [1..2] address lo/hi,
 * [3..4] command lo/hi, little endian. flags wird nicht gespeichert.
 *
 * RAM und EEPROM bleiben zusammen. Schlaegt der EEPROM-Schreibzugriff fehl -- moeglicherweise
 * mitten in den 5 Byte --, wird der RAM-Spiegel aus dem EEPROM nachgezogen, dieselbe Sorgfalt
 * wie nach einem abgebrochenen Lernvorgang weiter oben. Sonst reagierte die Uhr bis zum
 * naechsten Reset auf einen Code, der nirgends gespeichert ist.
 *
 * Rueckgabe: REMOTE_IR_SET_OK oder REMOTE_IR_SET_FAILED, beide in remote-ir.h. Hier bleibt es
 * still, damit dieser Pfad keine Logzeile auf die UART zum ESP legt; gemeldet wird beim
 * Aufrufer, und der kann die beiden Ursachen auseinanderhalten -- den Index prueft er vor dem
 * Aufruf selbst, ein Fehlschlag danach ist das EEPROM.
 *
 * Die Indexpruefung unten steht trotzdem, als zweite Verteidigungslinie. Heute loest sie
 * niemand aus; ein kuenftiger zweiter Aufrufer haette sie sonst nicht, und ein Schreibzugriff
 * neben irmp_data_array waere ein stiller Speicherfehler.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
remote_ir_set_code (uint_fast8_t idx, uint_fast8_t protocol, uint_fast16_t address, uint_fast16_t command)
{
    uint8_t         packed_irmp_data[PACKED_IRMP_DATA_SIZE];
    IRMP_DATA       saved_irmp_data;
    uint_fast16_t   start_addr;
    uint_fast8_t    rtc = REMOTE_IR_SET_FAILED;

    if (idx >= N_REMOTE_IR_CMDS)
    {
        return REMOTE_IR_SET_FAILED;                                                    // nichts angefasst, auch nicht der RAM-Spiegel
    }

    if (eep_is_up)
    {
        saved_irmp_data             = irmp_data_array[idx];

        irmp_data_array[idx].protocol   = protocol;
        irmp_data_array[idx].address    = address;
        irmp_data_array[idx].command    = command;
        irmp_data_array[idx].flags      = 0;

        packed_irmp_data[0]         = irmp_data_array[idx].protocol;
        packed_irmp_data[1]         = irmp_data_array[idx].address & 0xFF;
        packed_irmp_data[2]         = irmp_data_array[idx].address >> 8;
        packed_irmp_data[3]         = irmp_data_array[idx].command & 0xFF;
        packed_irmp_data[4]         = irmp_data_array[idx].command >> 8;

        start_addr                  = EEPROM_DATA_OFFSET_IRMP_DATA + (uint_fast16_t) idx * PACKED_IRMP_DATA_SIZE;

        if (eep_write (start_addr, (uint8_t *) &packed_irmp_data, PACKED_IRMP_DATA_SIZE))
        {
            rtc = REMOTE_IR_SET_OK;
        }
        else
        {
            rtc = REMOTE_IR_SET_FAILED;

            if (eep_read (start_addr, (uint8_t *) &packed_irmp_data, PACKED_IRMP_DATA_SIZE))
            {
                irmp_data_array[idx].protocol   = packed_irmp_data[0];
                irmp_data_array[idx].address    = packed_irmp_data[1] | (packed_irmp_data[2] << 8);
                irmp_data_array[idx].command    = packed_irmp_data[3] | (packed_irmp_data[4] << 8);
                irmp_data_array[idx].flags      = 0;
            }
            else
            {
                irmp_data_array[idx] = saved_irmp_data;                                     // auch das Lesen scheitert: alten Stand zurueck
            }
        }
    }

    return rtc;
}
