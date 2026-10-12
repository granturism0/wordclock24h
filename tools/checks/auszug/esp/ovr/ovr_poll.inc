    if (Serial.hasOverrun () && esp_uart_overruns < 0xFFFFFFFFUL)           // E.3: zuerst abfragen (loescht das
    {                                                                       // Merkmal), dann saettigend zaehlen
        esp_uart_overruns++;
    }

