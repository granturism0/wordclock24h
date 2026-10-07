/* Pruefstand A24 (S.19d, AKK.6): tables_fill_words() und tables_idx_ok() aus src/tables/tables.c.
 * (1) Wortindex 255 (und 128, 200) wird abgewiesen: kein Byte hinter words[WP_COUNT] veraendert.
 * (2) Gueltige Tabelle: Ergebnis fuer alle Stunden/Minuten wird als Pruefsumme ausgegeben; sie muss
 *     zwischen alter und neuer Fassung byte-gleich sein (run.sh vergleicht).
 * -DZIELTYPEN: uint_fast* 32 Bit wie arm-none-eabi. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pruefstand.h"
#ifdef ZIELTYPEN
#define uint_fast8_t    uint32_t
#define uint_fast16_t   uint32_t
#define int_fast8_t     int32_t
typedef char zp_fast_pruefung[(sizeof (uint_fast8_t) == 4 && sizeof (uint_fast16_t) == 4) ? 1 : -1];
#endif
#include "tables.h"
TABLES_GLOBALS tables;
static int logs;
static int log_printf (const char * f, ...) { (void) f; logs++; return 0; }
#include SNIPPET

static uint8_t  buf[512];                       /* words = buf + 128: davor und dahinter Waechterbytes */
#define CANARY  0xA5
static int fails, cases;
#define CHECK(name, cond) do { cases++; if (cond) printf ("  OK    %s\n", name); else { fails++; printf ("  FEHL  %s\n", name); } } while (0)

static int
waechter_heil (void)
{
    int i;
    for (i = 0; i < 128; i++) if (buf[i] != CANARY) return 0;
    for (i = 256; i < 512; i++) if (buf[i] != CANARY) return 0;
    return 1;
}

static void
gueltige_tabelle (void)
{
    int h, m, i;
    memset (&tables, 0, sizeof (tables));
    tables.hour_count = HOUR_COUNT; tables.minute_count = MINUTE_COUNT; tables.max_minute_words = MAX_MINUTE_WORDS;
    for (m = 0; m < MINUTE_COUNT; m++)
    {
        tables.minutes[m].flags = (m % 3 == 1) ? MDF_HOUR_OFFSET_1 : 0;
        for (i = 0; i < 1 + (m % MAX_MINUTE_WORDS); i++) tables.minutes[m].word_idx[i] = (uint8_t) (1 + (m * 7 + i * 13) % 127);
    }
    for (h = 0; h < HOUR_COUNT; h++)
    {
        tables.hours[h][0] = (uint8_t) (1 + h);
        if (h == 1)      { tables.hours[h][1] = WP_IF_MINUTE_IS_0; tables.hours[h][2] = 40; tables.hours[h][3] = 41; tables.hours[h][4] = 99; }
        else if (h == 0) { tables.hours[h][1] = WP_IF_HOUR_IS_0;   tables.hours[h][2] = 50; tables.hours[h][3] = 51; }
        else             { tables.hours[h][1] = (uint8_t) (60 + h); }
    }
    tables.complete = 1;
}

int
main (void)
{
    uint32_t sum = 0;
    unsigned hh, mm, i;
    uint8_t * words = buf + 128;

    printf ("[A24] tables_fill_words\n");
    gueltige_tabelle ();
    for (hh = 0; hh < 24; hh++)
        for (mm = 0; mm < MINUTE_COUNT; mm++)
        {
            memset (buf, CANARY, sizeof (buf));
            tables_fill_words (words, hh, mm);
            for (i = 0; i < WP_COUNT; i++) sum = sum * 31 + words[i];
        }
    printf ("  Pruefsumme gueltige Tabelle: %08x\n", (unsigned) sum);
    CHECK ("gueltige Tabelle: nichts abgewiesen", logs == 0);

    {
        static const uint8_t boese[] = { 128, 200, 255 };
        for (i = 0; i < 3; i++)
        {
            char n[80];
            gueltige_tabelle (); logs = 0;
            tables.minutes[2].word_idx[0] = boese[i];            /* Minutenzweig */
            tables.hours[3][1] = boese[i];                       /* Stundenzweig */
            tables.hours[1][4] = boese[i];                       /* Markerzweig: WP_IF_MINUTE_IS_0, mm != 0 -> idx + 2 ... */
            tables.hours[1][2] = boese[i];                       /* ... und mm == 0 -> idx + 1 */
            for (hh = 0; hh < 24; hh++)
                for (mm = 0; mm < MINUTE_COUNT; mm++)
                {
                    memset (buf, CANARY, sizeof (buf));
                    tables_fill_words (words, hh, mm);
                    if (! waechter_heil ()) { hh = 99; break; }
                }
            snprintf (n, sizeof (n), "Wortindex %3u: kein Schreibzugriff ausserhalb words[]", boese[i]);
            CHECK (n, hh != 100);
            snprintf (n, sizeof (n), "Wortindex %3u: abgewiesen und gemeldet", boese[i]);
            CHECK (n, logs > 0);
        }
    }
    printf ("\n%d Faelle, %d fehlgeschlagen\n", cases, fails);
    return fails ? 1 : 0;
}
