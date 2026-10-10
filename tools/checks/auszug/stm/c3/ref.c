/* Referenz "alt" fuer Pruefstand C3: eeprom_write() im M-Stand, wortgleich aus
 * release/3.2.23-3.2.28-1.4.94 (Commit dbf2c4013e60), src/eeprom/eeprom.c ab Zeile 218.
 * Eingefroren, damit die Gegenprobe (--gegen) nicht gegen sich selbst vergleicht. */
#line 218 "eeprom.c@release/3.2.23-3.2.28-1.4.94"
uint_fast8_t
eeprom_write (uint_fast16_t start_addr, uint8_t * buffer, uint_fast16_t cnt)
{
    uint_fast8_t rtc = 1;                                   // cnt == 0 ist kein Fehler; vorher war rtc hier uninitialisiert
    uint_fast16_t   m_a  = start_addr;                      // Messzeile: Werte vor der Schleife festhalten
    uint_fast16_t   m_n  = cnt;
    uint_fast16_t   m_z  = 0;
    uint_fast16_t   m_d  = esp8266_uart_rxdrops ();
    uint32_t        m_t  = diag_ticks ();

    if (eeprom_is_up)
    {
        // we must write every single byte, because we have to wait 15ms every cycle
        while (cnt--)
        {
            uint8_t current;

            if (i2c_read (eeprom_addr, start_addr, 1, &current, 1) == I2C_OK && current == *buffer)
            {
                rtc = 1;                                    // Byte steht bereits so im EEPROM: kein Schreibzyklus, keine 15 ms
            }
            else
            {
                if (i2c_write (eeprom_addr, start_addr, 1, buffer, 1) == I2C_OK)
                {
                    rtc = 1;
                }
                else
                {
                    rtc = 0;
                    break;
                }
                eeprom_waitstates ();
                m_z++;
            }

            start_addr++;
            buffer++;
        }
    }
    else
    {
        rtc = 0;
    }

    if (m_z)
    {
        log_printf ("eep a=%u n=%u z=%u ms=%lu d=%u/%u\r\n", (unsigned int) m_a, (unsigned int) m_n, (unsigned int) m_z,
                    (unsigned long) ((diag_ticks () - m_t) / diag_ticks_per_ms), (unsigned int) m_d, (unsigned int) esp8266_uart_rxdrops ());
    }
    return rtc;
}
