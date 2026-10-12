/*-------------------------------------------------------------------------------------------------------------------------------------------
 * i2c.c - I2C routines
 *
 * Ports/Pins:
 *
 *  +------+------------------+---------------+---------------+
 *  |      | STM32F4x1 Nucleo | STM32F407VE   | STM32F103C8T6 |
 *  +------+------------------+---------------+---------------+
 *  | SCL  | I2C3  PA8        | I2C1  PB8     | I2C1  PB6     |
 *  | SDA  | I2C3  PC9        | I2C1  PB9     | I2C1  PB7     |
 *  +------+------------------+---------------+---------------+
 *
 * Copyright (c) 2014-2026 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#include "i2c.h"
#include "delay.h"
#include "io.h"
#include "log.h"

#define I2C_TIMEOUT         5                                       // timeout: 5 msec
#define I2C_TIMEOUT_TICKS   (I2C_TIMEOUT * (1000 / DELAY_RESOLUTION_5_US))   // A60: Frist in SysTick-Ticks (5 us)

#if defined (BLACK_BOARD)

#define PERIPH_SCL_PORT     RCC_AHB1Periph_GPIOB
#define SCL_PORT            GPIOB                                   // SCL PB8
#define SCL_PIN             GPIO_Pin_8
#define SCL_PIN_SOURCE      GPIO_PinSource8

#define PERIPH_SDA_PORT     RCC_AHB1Periph_GPIOB
#define SDA_PORT            GPIOB                                   // SDA PB9
#define SDA_PIN             GPIO_Pin_9
#define SDA_PIN_SOURCE      GPIO_PinSource9

#define I2C_CHANNEL         I2C1
#define RCC_PERIPH_I2C      RCC_APB1Periph_I2C1
#define GPIO_AF_I2C         GPIO_AF_I2C1

#elif defined (NUCLEO_BOARD)

#define PERIPH_SCL_PORT     RCC_AHB1Periph_GPIOA
#define SCL_PORT            GPIOA                                   // SCL PA8
#define SCL_PIN             GPIO_Pin_8
#define SCL_PIN_SOURCE      GPIO_PinSource8

#define PERIPH_SDA_PORT     RCC_AHB1Periph_GPIOC
#define SDA_PORT            GPIOC                                   // SDA PC9
#define SDA_PIN             GPIO_Pin_9
#define SDA_PIN_SOURCE      GPIO_PinSource9

#define I2C_CHANNEL         I2C3
#define RCC_PERIPH_I2C      RCC_APB1Periph_I2C3
#define GPIO_AF_I2C         GPIO_AF_I2C3

#elif defined (BLACKPILL_BOARD)

#define PERIPH_SCL_PORT     RCC_AHB1Periph_GPIOB
#define SCL_PORT            GPIOB                                   // SCL PB6
#define SCL_PIN             GPIO_Pin_6
#define SCL_PIN_SOURCE      GPIO_PinSource6

#define PERIPH_SDA_PORT     RCC_AHB1Periph_GPIOB
#define SDA_PORT            GPIOB                                   // SDA PB7
#define SDA_PIN             GPIO_Pin_7
#define SDA_PIN_SOURCE      GPIO_PinSource7

#define I2C_CHANNEL         I2C1
#define RCC_PERIPH_I2C      RCC_APB1Periph_I2C1
#define GPIO_AF_I2C         GPIO_AF_I2C1

#elif defined (BLUEPILL_BOARD)

#define PERIPH_SCL_SDA_PORT RCC_APB2Periph_GPIOB
#define SCL_PORT            GPIOB                                   // SCL PB6
#define SCL_PIN             GPIO_Pin_6

#define SDA_PORT            GPIOB                                   // SDA PB7
#define SDA_PIN             GPIO_Pin_7

#define I2C_CHANNEL         I2C1
#define RCC_PERIPH_I2C      RCC_APB1Periph_I2C1

#else
#error STM32 undefined
#endif

static uint32_t             i2c_clockspeed;
uint_fast8_t                i2c_nack_quiet;                     // A73: NACK still (Adresssuche in eeprom_init), sonst 0
static uint32_t             i2c_last_clockspeed;                // A60/M1: Dateiebene, damit die BUSY-Frist i2c_init() erzwingen kann

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * init i2c bus
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
i2c_init_i2c (void)
{
    I2C_InitTypeDef  i2c;

    I2C_StructInit (&i2c);

    I2C_DeInit(I2C_CHANNEL);

    i2c.I2C_Mode                  = I2C_Mode_I2C;
    i2c.I2C_DutyCycle             = I2C_DutyCycle_2;
    i2c.I2C_OwnAddress1           = 0x00;
    i2c.I2C_Ack                   = I2C_Ack_Enable;
    i2c.I2C_AcknowledgedAddress   = I2C_AcknowledgedAddress_7bit;
    i2c.I2C_ClockSpeed            = i2c_clockspeed;

    I2C_Init (I2C_CHANNEL, &i2c);
    I2C_Cmd (I2C_CHANNEL, ENABLE);

    // fm 2017-10-18 +
    // I2C_ClearFlag(I2C_Module, I2C_FLAG_AF);
    // I2C_ClearFlag(I2C_Module, I2C_FLAG_ARLO);
    // I2C_ClearFlag(I2C_Module, I2C_FLAG_BERR);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * handle timeout
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
i2c_handle_timeout (void)
{
    /* G2-Korrektur (S3.2): Ein NACK ist kein Busfehler. eep_init() sucht das EEPROM auf 8 Adressen, 7 davon
     * leer -- das gab beim Start 7 Zeilen "i2c: frist abgelaufen".
     * Review S3.13 M2 (A73): STILL ist ein NACK aber nur waehrend dieser Suche (i2c_nack_quiet, gesetzt von
     * eeprom_init). Im Betrieb ist ein NACK ein Signal -- RTC oder EEPROM sind vom Bus verschwunden, etwa durch
     * einen Wackelkontakt, und rtc.c/eeprom.c loggen selbst nicht. Dann wieder die Zeile wie in 3.2.30, aber
     * weiterhin OHNE Freitakten: Ein NACK ist kein haengender Bus. AF (Acknowledge Failure) setzt die
     * Hardware, wenn auf ein gesendetes Byte kein ACK kommt; beim Adressbyte am 9. Takt, also nach rund
     * 90 us bei 100 kHz, lange vor der Frist. Ein Hang mit SDA low zeigt KEIN AF: low im ACK-Takt ist ein
     * ACK, und bei belegtem Bus kommt es gar nicht erst zum Adressbyte (SB-Frist). AF muss VOR dem SWRST
     * unten gelesen werden, der loescht SR1 -- dadurch kann auch kein altes AF stehen bleiben. Ablauf sonst
     * gleich (STOP, Reset), und die Frist bleibt ein Fehler fuer den Aufrufer. */
    uint_fast8_t    nack = I2C_GetFlagStatus (I2C_CHANNEL, I2C_FLAG_AF);

    /* A60 / AKI.3: eine Zeile je Fristablauf (Flag- und BUSY-Warte laufen hier zusammen), fester Text fuer
     * die Logwache. Sicher: I2C laeuft erst nach log_init() (rtc_init/eep_init in main()) und nie aus einer ISR. */
    if (! (nack && i2c_nack_quiet))
    {
        log_puts ("i2c: frist abgelaufen\r\n");
    }

    I2C_GenerateSTOP (I2C_CHANNEL, ENABLE);
    I2C_SoftwareResetCmd (I2C_CHANNEL, ENABLE);
    I2C_SoftwareResetCmd (I2C_CHANNEL, DISABLE);

    I2C_DeInit (I2C_CHANNEL);
    i2c_init_i2c ();

    /* M1 (Review S2, Entscheidung des Nutzers "Bus beim Fristablauf freitakten"), fuer JEDE Frist: Haelt ein
     * Slave SDA low, hilft der Peripherie-Reset oben nicht, und jeder folgende Zugriff scheitert schon am
     * START (SB-Frist, RM0008: bei belegtem Bus kein START). Bisher taktete erst der Neustart nach dem
     * Watchdog frei (i2c_init() -> i2c_reset_bus()). Jetzt hier, aber nur wenn der Bus noch belegt ist --
     * eine gewoehnliche Flag-Frist ohne Busstoerung kostet so keine 20 ms.
     *
     * "Belegt" = BUSY gesetzt ODER SDA liest low. Die Leitung wird ueber das GPIO-Eingangsregister gelesen:
     * Laut Referenzhandbuch (RM0008/RM0383, GPIO, Alternate-Function-Konfiguration) tastet es den Pin auch im
     * AF-Open-Drain-Modus ab. BUSY allein genuegt nicht: SWRST/DeInit loeschen SR2, und RM0008 sagt zu BUSY
     * nur "set by hardware on detection of SDA or SCL low" -- ob das Modul nach dem Reset einen SCHON low
     * liegenden SDA wieder erkennt, ist dort nicht zugesichert und am Geraet nicht belegt. Der Pegel ist der
     * Zustand selbst; BUSY bleibt als zweite Angabe.
     *
     * i2c_init() erzwingen (nicht i2c_reset_bus() allein): Es taktet 9 Takte frei und stellt die Pins danach
     * wieder auf AF. Keine Rekursion -- i2c_init() wartet auf kein Flag. Seine delay_msec() setzen
     * delay_counter selbst; die Frist ist abgelaufen, der naechste Zugriff setzt sie neu. Bis rund 20 ms,
     * nur nach log_init()/delay_init() und nie aus einer ISR. */
    if (! nack && (I2C_GetFlagStatus (I2C_CHANNEL, I2C_FLAG_BUSY) || GPIO_ReadInputDataBit (SDA_PORT, SDA_PIN) == Bit_RESET))
    {
        i2c_last_clockspeed = 0;
        i2c_init (i2c_clockspeed);
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * wait for flags
 *
 * A60 / L359 (specs/paket-2026-10-11, design.md 8, Ent-15 nur Fristen): eng pollen statt je Durchlauf
 * delay_msec(1). Bisher kostete JEDES Flag mindestens 1 ms, auch wenn es nach wenigen Mikrosekunden kam
 * -- daher die rund 24 ms je EEPROM-Zyklus statt 16 ms Schreibzeit (L358/L359). Die Frist bleibt 5 ms.
 *
 * Frist ueber delay_counter: KEIN freilaufender Zaehler, sondern ein Abwaertszaehler, den
 * SysTick_Handler() (delay.c) alle 5 us bis 0 herunterzaehlt -- dasselbe Verfahren wie delay_msec(),
 * nur dass hier waehrenddessen das Flag abgefragt wird. Voraussetzung, am Code geprueft (S2.1): keine
 * ISR ruft delay_*() oder schreibt delay_counter (TIM2, DMA der LED-Treiber, UART, XPT2046, Fault-
 * Handler); aufgerufen wird i2c nur aus dem Hauptloop bzw. aus main() nach delay_init().
 * I2C_TIMEOUT_TICKS rechnet mit der Aufloesung, die main.c und onewire.c setzen (5 us); aendert sich
 * die, aendert sich die Frist mit.
 *
 * return values:
 * == 1  OK, got flag
 * == 0  timeout
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
i2c_wait_for_flags (uint32_t flag1, uint32_t flag2)
{
    delay_counter = I2C_TIMEOUT_TICKS;

    while (! I2C_GetFlagStatus (I2C_CHANNEL, flag1) || (flag2 && ! I2C_GetFlagStatus (I2C_CHANNEL, flag2)))
    {
        if (delay_counter == 0)
        {
            i2c_handle_timeout ();
            return 0;
        }
    }

    return 1;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * wait until bus is free after STOP (A60 / L360)
 *
 * Hier stand an zwei Stellen (i2c_read, i2c_write) eine Schleife auf BUSY OHNE Frist. Haengt der Bus,
 * stand der Hauptloop bis zum Watchdog-Reset -- und VOR watchdog_init() (rtc_init/eep_init beim Start)
 * fuer immer. Jetzt dieselbe Frist wie oben, danach derselbe Abbruch.
 *
 * return values:
 * == 1  OK, bus free
 * == 0  timeout
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
i2c_wait_not_busy (void)
{
    delay_counter = I2C_TIMEOUT_TICKS;

    while (I2C_GetFlagStatus (I2C_CHANNEL, I2C_FLAG_BUSY))
    {
        if (delay_counter == 0)
        {
            i2c_handle_timeout ();                                          // Zeile und Freitakten wie bei jeder Frist
            return 0;
        }
    }

    return 1;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * send address
 *
 * return values:
 * ==  0 I2C_OK
 *  <  0 Error
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static int_fast16_t
i2c_send_address (uint_fast8_t slave_addr, uint_fast16_t addr, uint_fast8_t is_16_bit_addr, uint_fast8_t disable_ack)
{
    I2C_GenerateSTART(I2C_CHANNEL, ENABLE);

    if (! i2c_wait_for_flags (I2C_FLAG_SB, 0))
    {
        return I2C_ERROR_NO_FLAG_SB;
    }

    if (disable_ack)
    {
        I2C_AcknowledgeConfig(I2C_CHANNEL, DISABLE);                               // ACK disable
    }
    else
    {
        I2C_AcknowledgeConfig(I2C_CHANNEL, ENABLE);                                // ACK enable
    }


    I2C_Send7bitAddress(I2C_CHANNEL, slave_addr, I2C_Direction_Transmitter);        // send slave address (transmitter)

    if (! i2c_wait_for_flags (I2C_FLAG_ADDR, 0))
    {
        return I2C_ERROR_NO_FLAG_ADDR;
    }

    I2C_CHANNEL->SR2;                                                              // clear ADDR-Flag

    if (! i2c_wait_for_flags (I2C_FLAG_TXE, 0))
    {
        return I2C_ERROR_NO_FLAG_TXE;
    }

    if (is_16_bit_addr)
    {
        uint8_t addrh;
        uint8_t addrl;

        addrh = (addr >> 8);
        addrl = (addr & 0xFF);

        I2C_SendData (I2C_CHANNEL, addrh);                                         // send upper byte of address
        I2C_SendData (I2C_CHANNEL, addrl);                                         // send lower byte of address
    }
    else
    {
        I2C_SendData(I2C_CHANNEL, addr);                                           // send address
    }

    return I2C_OK;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * reset I2C bus
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
i2c_reset_bus (void)
{
    uint_fast8_t        idx;

#if defined (STM32F4XX)
    RCC_AHB1PeriphClockCmd(PERIPH_SCL_PORT, ENABLE);                // for SCL
    RCC_AHB1PeriphClockCmd(PERIPH_SDA_PORT, ENABLE);                // for SDA
#elif defined (STM32F10X)
    RCC_APB2PeriphClockCmd(PERIPH_SCL_SDA_PORT, ENABLE);            // for SCL & SDA
#else
#error STM32 undefined
#endif

    GPIO_SET_PIN_OUT_OD(SCL_PORT, SCL_PIN, GPIO_Speed_2MHz);
    GPIO_SET_PIN_IN_NOPULL(SDA_PORT, SDA_PIN, GPIO_Speed_2MHz);

    if (GPIO_ReadInputDataBit(SDA_PORT, SDA_PIN) == Bit_RESET)      // SDA low?
    {                                                               // yes...
        GPIO_RESET_BIT(SDA_PORT, SCL_PIN);                          // try to release it here....
        delay_msec (1);
    }

    for (idx = 0; idx < 9; idx++)
    {
        GPIO_SET_BIT(SCL_PORT, SCL_PIN);
        delay_msec (1);

        if (GPIO_ReadInputDataBit(SDA_PORT, SDA_PIN) == Bit_SET)    // look for SDA = HIGH
        {                                                           // step 2!
            break;
        }

        GPIO_RESET_BIT(SCL_PORT, SCL_PIN);
        delay_msec (1);
    }

    GPIO_SET_BIT(SCL_PORT, SCL_PIN);
    delay_msec (1);

    if (GPIO_ReadInputDataBit(SDA_PORT, SDA_PIN) == Bit_RESET)      // SDA still low
    {                                                               // yes...
        log_message ("error: cannot reset I2C bus");
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * initialize I2C
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
i2c_init (uint32_t clockspeed)
{
    GPIO_InitTypeDef    gpio;

    if (i2c_last_clockspeed == clockspeed)                          // if already called with same clockspeed, return
    {
        return;
    }

    i2c_last_clockspeed = clockspeed;

    I2C_DeInit(I2C_CHANNEL);

    i2c_reset_bus ();                                               // if 2i2c bus hangs, reset it by toggling SCK 9 times

    GPIO_StructInit (&gpio);

#if defined (STM32F4XX)
    RCC_AHB1PeriphClockCmd(PERIPH_SCL_PORT, ENABLE);                // for SCL
    RCC_AHB1PeriphClockCmd(PERIPH_SDA_PORT, ENABLE);                // for SDA

    // I2C reset
    RCC_APB1PeriphResetCmd(RCC_PERIPH_I2C, ENABLE);
    RCC_APB1PeriphResetCmd(RCC_PERIPH_I2C, DISABLE);

    GPIO_PinAFConfig(SCL_PORT, SCL_PIN_SOURCE, GPIO_AF_I2C);        // SCL: PA8 / PB8
    GPIO_PinAFConfig(SDA_PORT, SDA_PIN_SOURCE, GPIO_AF_I2C);        // SDA: PC9 / PB9

    gpio.GPIO_Mode  = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_OD;                                // OpenDrain!
    gpio.GPIO_PuPd  = GPIO_PuPd_UP; // GPIO_PuPd_NOPULL;

    gpio.GPIO_Pin = SCL_PIN;                                        // SCL pin
    GPIO_Init(SCL_PORT, &gpio);

    gpio.GPIO_Pin = SDA_PIN;                                        // SDA pin
    GPIO_Init(SDA_PORT, &gpio);

    RCC_APB1PeriphClockCmd(RCC_PERIPH_I2C, ENABLE);                 // new: enable I2C Peripheral Clock *after* initializing GPIO

#elif defined (STM32F10X)
    RCC_APB2PeriphClockCmd(PERIPH_SCL_SDA_PORT, ENABLE);            // for SCL & SDA

    // I2C reset
    RCC_APB1PeriphResetCmd(RCC_PERIPH_I2C, ENABLE);
    RCC_APB1PeriphResetCmd(RCC_PERIPH_I2C, DISABLE);

    gpio.GPIO_Pin   = SCL_PIN | SDA_PIN;                            // SCL=PB6, SDA=PB7
    gpio.GPIO_Mode  = GPIO_Mode_AF_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SCL_PORT, &gpio);

    RCC_APB1PeriphClockCmd(RCC_PERIPH_I2C, ENABLE);                 // new: enable I2C Peripheral Clock *after* initializing GPIO
#else
#error STM32 undefined
#endif

    i2c_clockspeed = clockspeed;
    i2c_init_i2c ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read one or more bytes
 *
 * return values:
 * ==  0 OK
 *  <  0 Error
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
int_fast16_t
i2c_read (uint_fast8_t slave_addr, uint_fast16_t addr, uint_fast8_t is_16_bit_addr, uint8_t * data, uint_fast16_t cnt)
{
    uint_fast16_t   n;
    int_fast16_t    rtc;

    if ((rtc = i2c_send_address (slave_addr, addr, is_16_bit_addr, cnt == 1 ? 1 : 0)) != I2C_OK)
    {
        return rtc;
    }

    if (! i2c_wait_for_flags (I2C_FLAG_TXE, I2C_FLAG_BTF))
    {
        return I2C_ERROR_NO_TXE_OR_BTF;
    }

    I2C_GenerateSTART(I2C_CHANNEL, ENABLE);                                        // start sequence

    if (! i2c_wait_for_flags (I2C_FLAG_SB, 0))
    {
        return I2C_ERROR_NO_FLAG_SB2;
    }

    I2C_Send7bitAddress(I2C_CHANNEL, slave_addr, I2C_Direction_Receiver);          // send slave address (receiver)

    if (! i2c_wait_for_flags (I2C_FLAG_ADDR, 0))
    {
        return I2C_ERROR_NO_FLAG_ADDR2;
    }

    I2C_CHANNEL->SR2;                                                              // clear ADDR flag

    for (n = 0; n < cnt; n++)                                               // read all data
    {
        if (n + 1 == cnt)
        {
            I2C_AcknowledgeConfig(I2C_CHANNEL, DISABLE);                           // ACK disable
            I2C_GenerateSTOP(I2C_CHANNEL, ENABLE);                                 // stop sequence

            if (! i2c_wait_not_busy ())                                             // A60: bisher ohne Frist
            {
                return I2C_ERROR_BUSY;
            }
        }

        if (! i2c_wait_for_flags (I2C_FLAG_RXNE, 0))
        {
            return I2C_ERROR_NO_FLAG_RXNE;
        }

        data[n] = I2C_ReceiveData(I2C_CHANNEL);                                    // read data
    }

    I2C_AcknowledgeConfig(I2C_CHANNEL, ENABLE);                                    // ACK enable

    return I2C_OK;
}


/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write one or more bytes
 *
 * return values:
 * ==  0 I2C_OK
 *  <  0 Error
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
int_fast16_t
i2c_write (uint_fast8_t slave_addr, uint_fast16_t addr, uint_fast8_t is_16_bit_addr, uint8_t * data, uint_fast16_t cnt)
{
    uint8_t         value;
    uint_fast16_t   n;
    int_fast16_t    rtc;

    if ((rtc = i2c_send_address (slave_addr, addr, is_16_bit_addr, 0)) != I2C_OK)
    {
        return rtc;
    }

    if (! i2c_wait_for_flags (I2C_FLAG_TXE, 0))
    {
        return I2C_ERROR_NO_FLAG_TXE;
    }

    for (n = 0; n < cnt; n++)                                               // send all data
    {
        value = *data++;                                                    // read data from buffer

        I2C_SendData(I2C_CHANNEL, value);                                   // send data

        if (! i2c_wait_for_flags (I2C_FLAG_TXE, I2C_FLAG_BTF))
        {
            return I2C_ERROR_NO_TXE_OR_BTF;
        }
    }

    I2C_GenerateSTOP(I2C_CHANNEL, ENABLE);                                  // stop sequence

    if (! i2c_wait_not_busy ())                                             // A60: bisher ohne Frist
    {
        return I2C_ERROR_BUSY;
    }

    return I2C_OK;
}
