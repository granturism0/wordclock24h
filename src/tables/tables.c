/*-------------------------------------------------------------------------------------------------------------------------------------------
 * tables.c - wordclock layout tables
 *
 * Copyright (c) 2014-2026 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdint.h>

#include "wclock24h-config.h"
#include "esp8266.h"
#include "display.h"
#include "tables.h"
#include "base.h"
#include "log.h"

#define TABLES_VERSION_MAGIC    0xFF                    // Magic of version header

TABLES_GLOBALS      tables;

static uint_fast8_t wp_count;
static uint_fast8_t current_mode;

/* Fortschrittsmarke des Tabellentransfers.
 *
 * Erhoeht wird sie von jeder angenommenen Tabellenzeile -- tabillu, tabh, tabm und tabt. Sie
 * zaehlt nichts Fachliches, sie beantwortet eine einzige Frage fuer den Warter in
 * display_set_display_mode(): Kommt die Kette noch voran, oder steht sie?
 *
 * Gebraucht wird das, weil die Kette JEDE Zeile einzeln anfordert: Jede empfangene Zeile
 * fordert die naechste an. Geht eine verloren, fordert niemand mehr etwas an, und der Transfer
 * steht still, ohne dass es jemand merkt (BEFUNDE.md, L146). Eine feste Gesamtzeit als
 * Abbruchbedingung muesste den langsamsten denkbaren Transfer abdecken und waere damit fuer den
 * Stillstandsfall unbrauchbar lang. Der Stillstand selbst ist das Signal.
 *
 * Umlaufend und ohne Bedeutung des Absolutwerts: Verglichen wird nur auf Ungleichheit.
 */
static uint_fast16_t tables_rx_cnt;

uint_fast16_t
tables_progress (void)
{
    return tables_rx_cnt;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * tables_idx_ok () - Indexpruefung fuer die empfangenen Tabellenzeilen
 *
 * Gemeinsame Pruefhilfe fuer tabillu, tabt, tabh und tabm. Die Schutzwirkung aus BEFUNDE.md L145
 * ist unveraendert: gleiche Bedingung, gleicher Abbruch, gleiche Diagnosezeile. Geaendert hat
 * sich nur, dass Pruefcode und Formatzeichenkette genau einmal im Flash liegen statt vier- bzw.
 * fuenfmal -- der F103 stand mit den Inline-Pruefungen 168 Byte ueber der Flashgrenze
 * (BEFUNDE.md, L165).
 *
 * Die Meldung ist aus demselben Grund kurz: Jede Zeichenkette kostet Flash. "tabh idx 200>12"
 * nennt Quelle, Wert und Grenze und sagt beim Debuggen damit alles, was der ganze Satz sagte.
 *
 * noinline ist Absicht und kein Stilmittel: Ohne das Attribut kopiert der Optimierer die
 * Funktion an jede Aufrufstelle zurueck, und die Ersparnis ist weg.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static __attribute__((noinline)) uint_fast8_t
tables_idx_ok (uint_fast8_t idx, uint_fast8_t limit, const char * was)
{
    uint_fast8_t    ok = 1;

    if (idx >= limit)
    {
        log_printf ("%s idx %u>%u\r\n", was, (unsigned int) idx, (unsigned int) limit);
        ok = 0;
    }

    return ok;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * tables_init () - initialize tables
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
tables_init (void)
{
    esp8266_send_cmd ("tabinfo", (const char *) NULL, 1);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * tables_get () - get a layout table
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
tables_get (uint_fast8_t mode)
{
    char            buf[16];

    current_mode = mode;
    tables.complete = 0;
    sprintf (buf, "%d\",\"0", current_mode);
    esp8266_send_cmd ("tabh", buf, 1);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * tables_get () - get temperature table (only WCLOCK24H)
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#if WCLOCK24H == 1
static void
tables_get_temperature_table (void)
{
    char            buf[16];

    sprintf (buf, "%d\",\"0", tables.modes_count - 1);
    esp8266_send_cmd ("tabt", buf, 1);
}
#endif

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * tables_tabinfo () - get tables info data
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
tables_tabinfo (char * info)
{
    uint_fast8_t    wc_rows;
    uint_fast8_t    wc_columns;

    wc_rows = htoi (info, 2);
    info += 2;
    wc_columns = htoi (info, 2);
    info += 2;

    if (wc_rows == WC_ROWS && wc_columns == WC_COLUMNS)
    {
        wp_count            = htoi (info, 2);
        info += 2;

        if (wp_count < WP_COUNT)
        {
            uint_fast8_t    version_magic;
            uint_fast8_t    version;
            uint_fast8_t    modes_count;
            uint_fast8_t    hour_count;
            uint_fast8_t    max_hour_words;
            uint_fast8_t    minute_count;
            uint_fast8_t    max_minute_words;

            /* Erst lesen, dann pruefen, dann uebernehmen -- nicht umgekehrt.
             *
             * Diese vier Werte sind die SCHLEIFENGRENZEN der Schreibzugriffe in tables_tabh(),
             * tables_tabm() und tables_tabt(). Bis zum 03.10.2026 wurden sie ungeprueft aus der
             * empfangenen Zeile uebernommen; wp_count war die einzige gepruefte Groesse
             * (BEFUNDE.md, L145). Ein einziges verlorenes Zeichen verschiebt alle Felder dieser
             * Zeile, und aus zwei Hexziffern wird ein Wert bis 255 -- bei hour_count waeren das
             * 255 statt 12 oder 24 Zeilen a MAX_HOUR_WORDS Byte neben das Array.
             *
             * Haetten sie bereits in tables.* gestanden, waeren sie auch nach dem Abbruch noch
             * als Schleifengrenze wirksam: tables.complete schuetzt tables_fill_words(), aber
             * nicht eine spaeter doch noch eintreffende tabh-Zeile.
             */
            version_magic           = htoi (info, 2);                                           // old: tables.it_is[0]
            info += 2;

            version                 = htoi (info, 2);                                           // old: tables.it_is[1]
            info += 2;

            modes_count             = htoi (info, 2);
            info += 2;

            hour_count              = htoi (info, 2);
            info += 2;

            max_hour_words          = htoi (info, 2);
            info += 2;

            minute_count            = htoi (info, 2);
            info += 2;

            max_minute_words        = htoi (info, 2);
            info += 2;

            if (hour_count <= HOUR_COUNT && max_hour_words <= MAX_HOUR_WORDS &&
                minute_count <= MINUTE_COUNT && max_minute_words <= MAX_MINUTE_WORDS)
            {
                tables.version_magic    = version_magic;
                tables.version          = version;
                tables.modes_count      = modes_count;
                tables.hour_count       = hour_count;
                tables.max_hour_words   = max_hour_words;
                tables.minute_count     = minute_count;
                tables.max_minute_words = max_minute_words;

                esp8266_send_cmd ("tabillu", "0", 1);
            }
            else
            {
                /* Kein erneutes Anfordern: Eine Zeile, die hier scheitert, kann auch dauerhaft
                 * falsch sein -- etwa ein 24-Stunden-Layout gegen einen 12-Stunden-Build. Ein
                 * Neuversuch liefe dann endlos. Den Neuanstoss macht der Warter in
                 * display_set_display_mode(), begrenzt und mit Zeitlimit.
                 */
                log_printf ("tabinfo out of range: h=%d/%d m=%d/%d\r\n",
                            hour_count, max_hour_words, minute_count, max_minute_words);
            }
        }
        else
        {
            log_printf ("wp_count %d exceeds WP_COUNT\r\n", wp_count);
        }
    }
    else
    {
        log_printf ("wc_rows/columns %d/%d not correct\r\n", wc_rows, wc_columns);
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * tables_tabillu () - store word illumination data
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
tables_tabillu (char * illu)
{
    uint_fast8_t    idx;

    idx = htoi (illu, 2);
    illu += 2;

    if (! tables_idx_ok (idx, wp_count, "tabillu"))                 // Index aus zwei Hexziffern: 0..255 gegen WP_COUNT = 128
    {
        return;                                                     // Kette steht -- siehe tables_tabinfo()
    }

    tables.illumination[idx].row = htoi (illu, 2);
    illu += 2;
    tables.illumination[idx].col = htoi (illu, 2);
    illu += 2;
    tables.illumination[idx].len = htoi (illu, 2);
    illu += 2;

    tables_rx_cnt++;
    idx++;

    if (idx < wp_count)
    {
        char buf[8];
        sprintf (buf, "%d", idx);

        esp8266_send_cmd ("tabillu", buf, 0);
    }
    else
    {
        if (display.display_mode >= tables.modes_count)
        {
            display.display_mode = 0;
            display_save_display_mode ();
        }

#if WCLOCK24H == 1
        tables_get_temperature_table ();                            // get temperature table, then current display table
#else
        tables_get (display.display_mode);                          // get current_display table now
#endif
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * tables_tabt () - store temperature table (only WCLOCK24H)
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#if WCLOCK24H == 1
void
tables_tabt (char * tabt)
{
    char            buf[16];
    uint_fast8_t    idx;
    uint_fast8_t    k;

    idx = htoi (tabt, 2);
    tabt += 2;

    if (! tables_idx_ok (idx, tables.minute_count, "tabt"))         // dieselbe Grenze wie tables.minutes: MINUTE_COUNT
    {
        return;
    }

    tables.temperature[idx].flags = htoi (tabt, 2);
    tabt += 2;

    for (k = 0; k < tables.max_minute_words; k++)
    {
        tables.temperature[idx].word_idx[k] = htoi (tabt, 2);
        tabt += 2;

        if (tables.temperature[idx].word_idx[k] == 0)
        {
            break;
        }
    }

    tables_rx_cnt++;
    idx++;

    if (idx < tables.minute_count)
    {
        sprintf (buf, "%d\",\"%d", tables.modes_count - 1, idx);
        esp8266_send_cmd ("tabt", buf, 0);
    }
    else
    {
        tables_get (display.display_mode);                              // get current_display table now
    }
}
#endif

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * tables_tabh () - store hour table
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
tables_tabh (char * tabh)
{
    char            buf[16];
    uint_fast8_t    idx;
    uint_fast8_t    k;
    uint_fast8_t    h_offset = 0;

    idx = htoi (tabh, 2);
    tabh += 2;

    if (! tables_idx_ok (idx, tables.hour_count, "tabh"))                                   // 0..255 gegen HOUR_COUNT
    {
        return;
    }

    if (tables.version_magic != TABLES_VERSION_MAGIC)                                       // old tables version
    {                                                                                       // insert IT and IS
        if (tables.version_magic > 0)
        {
            tables.hours[idx][h_offset] = tables.version_magic;                             // old: tbl_it_is[0]
            h_offset++;
        }

        if (tables.version > 0)
        {
            tables.hours[idx][h_offset] = tables.version;                                   // old: tbl_it_is[0]
            h_offset++;
        }
    }

    for (k = h_offset; k < tables.max_hour_words; k++)
    {
        tables.hours[idx][k] = htoi (tabh, 2);
        tabh += 2;

        /* Bis zum 03.10.2026 stand hier tables.hours[k] -- das ist die ADRESSE der Zeile k,
         * nie null, der break griff also nie (BEFUNDE.md, L145, Nebenbefund). Gelesen wurde
         * dadurch bis max_hour_words statt bis zum Endetoken; geschrieben wurde innerhalb der
         * Zeile, der Fehler blieb deshalb folgenlos bis auf die ueberzaehligen Hexziffern.
         */
        if (tables.hours[idx][k] == 0)
        {
            break;
        }
    }

    tables_rx_cnt++;
    idx++;

    if (idx < tables.hour_count)
    {
        sprintf (buf, "%d\",\"%d", current_mode, idx);
        esp8266_send_cmd ("tabh", buf, 0);
    }
    else
    {
        sprintf (buf, "%d\",\"0", current_mode);
        esp8266_send_cmd ("tabm", buf, 1);
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * tables_tabm () - store minute table
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
tables_tabm (char * tabm)
{
    char            buf[16];
    uint_fast8_t    idx;
    uint_fast8_t    k;

    idx = htoi (tabm, 2);
    tabm += 2;

    if (! tables_idx_ok (idx, tables.minute_count, "tabm"))         // 0..255 gegen MINUTE_COUNT
    {
        return;
    }

    tables.minutes[idx].flags = htoi (tabm, 2);
    tabm += 2;

    for (k = 0; k < tables.max_minute_words; k++)
    {
        tables.minutes[idx].word_idx[k] = htoi (tabm, 2);
        tabm += 2;

        if (tables.minutes[idx].word_idx[k] == 0)
        {
            break;
        }
    }

    tables_rx_cnt++;
    idx++;

    if (idx < tables.minute_count)
    {
        sprintf (buf, "%d\",\"%d", current_mode, idx);
        esp8266_send_cmd ("tabm", buf, 0);
    }
    else
    {
        if (tables.version_magic != TABLES_VERSION_MAGIC)                                           // old tables version
        {                                                                                           // version_magic und version sind in diesem
            if (tables.version_magic < WP_COUNT)                                                    // Zweig Wortindizes aus tabinfo, also 0..255
            {
                tables.illumination[tables.version_magic].len |= ILLUMINATION_FLAG_IT_IS;           // convert to newer version
            }

            if (tables.version < WP_COUNT)
            {
                tables.illumination[tables.version].len |= ILLUMINATION_FLAG_IT_IS;
            }
        }

        log_message ("tables complete");
        tables.complete = 1;
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * tables_fill_words () - fill words to display
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
tables_fill_words (uint8_t * words, uint_fast8_t hh, uint_fast8_t mm)
{
    uint_fast8_t            is_midnight = 0;
    const MINUTEDISPLAY *   tbl_minute;
    const uint8_t *         word_idx_p;
    uint_fast16_t           idx;
    uint_fast8_t            w;                                      // A24: Wortindex, an EINER Stelle je Schleife geprueft

    /* A24 / L162: Die Wortindizes kommen als Tabelle vom ESP, zwei Hexziffern, also 0..255, und
     * wurden ungeprueft in words[WP_COUNT] geschrieben -- ein Schreibzugriff bis 127 Byte hinter das
     * Feld des Aufrufers. tables_idx_ok() beim Transfer prueft nur den Zeilenindex, nicht die Werte.
     * Jetzt wird jeder Index vor dem Schreiben geprueft; ein ungueltiges Wort faellt weg, der Rest
     * der Anzeige bleibt. Die Meldung ist ungedrosselt wie beim Transfer: Die Funktion laeuft je
     * Anzeigeaufbau, nicht je Bildwechsel, und nur eine beschaedigte Tabelle loest sie aus.
     */
    if (tables.complete)
    {
        memset (words, 0, WP_COUNT);
        tbl_minute  = &(tables.minutes[mm]);

        for (idx = 0; idx < tables.max_minute_words && tbl_minute->word_idx[idx] != 0; idx++)
        {
            w = tbl_minute->word_idx[idx];

            if (tables_idx_ok (w, WP_COUNT, "word"))
            {
                words[w] = 1;
            }
        }

        if (tbl_minute->flags & MDF_HOUR_OFFSET_1)
        {
            hh += 1;                                                // correct hour offset
        }
        else if (tbl_minute->flags & MDF_HOUR_OFFSET_2)             // only used in jester mode
        {
            hh += 2;                                                // correct hour offset
        }

        if (hh == 0 || hh == 24)                                    // we have midnight
        {
            is_midnight = 1;
        }

        while (hh >= tables.hour_count)                             // hour: 25 -> 13 -> 01
        {
            hh -= tables.hour_count;
        }

        word_idx_p = tables.hours[hh];                              // get the hour words from hour table

        for (idx = 0; idx < MAX_HOUR_WORDS && word_idx_p[idx] != 0; idx++)
        {
            w = word_idx_p[idx];

            /* WP_IF_MINUTE_IS_0: "EIN UHR" statt "EINS UHR" um 01:00 und 13:00 -- bei Minute 0 das
             * Wort idx + 1, sonst idx + 2. WP_IF_HOUR_IS_0: "MINUIT" statt "MIDI" um 00:00 im
             * franzoesischen Modus -- um Mitternacht idx + 1, sonst idx + 2. Beides wie bisher, nur
             * als EIN berechneter Index, damit das Schreiben eine einzige Pruefung hat (A24).
             */
            if (w == WP_IF_MINUTE_IS_0 || w == WP_IF_HOUR_IS_0)
            {
                w = word_idx_p[idx + (((w == WP_IF_MINUTE_IS_0) ? (mm == 0) : is_midnight) ? 1 : 2)];
                idx += 2;
            }

            if (tables_idx_ok (w, WP_COUNT, "word"))
            {
                words[w] = 1;
            }
        }
    }

    return tables.complete;
}
