static void
esp_heap_log (void)
{
    static uint32_t    last_millis = 0;
    static uint_fast8_t     pending     = 1;                                // die erste Zeile sofort, nicht erst nach einer Minute

    if (pending || (millis () - last_millis) >= ESP_HEAP_LOG_INTERVAL)      // Differenzbildung ist ueberlaufsicher
    {
        pending     = 0;
        last_millis = millis ();

        /* EINMAL formatieren, ZWEIMAL ausgeben: Mitschnitt und API duerfen nicht
         * auseinanderlaufen - eine zweite Formatierung waere eine zweite Wahrheit.
         * Laengste Form: "- heap free=4294967295 max=4294967295 ovr=4294967295" = 52 Zeichen.
         * ovr= steht HINTEN: tools/mess-stabilitaet.sh liest "free=<n> max=<n>" und bleibt so gueltig.
         */
        char line[64];

        snprintf_P (line, sizeof (line), PSTR ("- heap free=%lu max=%lu ovr=%lu"),
                  (unsigned long) ESP.getFreeHeap (),
                  (unsigned long) ESP.getMaxFreeBlockSize (),
                  (unsigned long) esp_uart_overruns);

        Serial.println (line);
        Serial.flush ();
        stm32_log_append (line);                                            // C14 (L185): ohne diese Zeile bleibt
                                                                            // die Messung ueber /api/stm32_log
                                                                            // unsichtbar - das Praefix allein
                                                                            // genuegt nicht
    }
}
