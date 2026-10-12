/* Pruefstand L408 (a): Dimmtabellen senden 16 Elemente, nicht 64.
 * Echter Code: var_send_num8_array(), var_send_dimmed_display_colors(), var_send_dimmed_ambilight_colors()
 * aus vars.c (extract.py). var_send_buf() ist hier ein Mitschreiber. display bildet das Ende von
 * DISPLAY_GLOBALS nach (display.h): beide Tabellen, ticker_deceleration, date_ticker_format -- dahinter
 * ein Schutzbereich, damit ein Ueberlesen definierte Werte liefert statt undefiniertes Verhalten. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "pruefstand.h"
#include "vars.h"

typedef struct
{
    uint_fast8_t    dimmed_display_colors[MAX_BRIGHTNESS + 1];
    uint_fast8_t    dimmed_ambilight_colors[MAX_BRIGHTNESS + 1];
    uint_fast8_t    ticker_deceleration;
    uint8_t         date_ticker_format[DATE_TICKER_FORMAT_LEN];
} DISP_ENDE;
static struct { DISP_ENDE d; uint8_t schutz[2048]; } wrap;
#define display wrap.d

static char sent[256][32];
static int  n_sent, bad_idlen;
static uint_fast8_t var_send_buf (char * buf, uint_fast8_t idlen)
{
    if (idlen != 5) bad_idlen++;
    if (n_sent < 256) snprintf (sent[n_sent], sizeof sent[0], "%s", buf);
    n_sent++;
    return 1;
}
#include SNIPPET

static int faelle, fehl;
static void pruef (int ok, const char * w) { faelle++; if (! ok) { fehl++; printf ("  FEHL  %s\n", w); } }

static void tabelle (const char * name, void (*f) (void), int var, uint_fast8_t * tab, unsigned basis)
{
    char soll[32], was[160]; int i, n = MAX_BRIGHTNESS + 1, gleich = 1;
    for (i = 0; i < n; i++) tab[i] = (uint_fast8_t) (basis + i);
    n_sent = 0; bad_idlen = 0;
    f ();
    snprintf (was, sizeof was, "%s: %d Elemente gesendet, Soll %d (sizeof Element %u Byte)", name, n_sent, n, (unsigned) sizeof tab[0]);
    pruef (n_sent == n, was);
    for (i = 0; i < n && i < n_sent; i++)
    {
        snprintf (soll, sizeof soll, "n%02x%02x%02x", var, i, basis + i);
        if (strcmp (sent[i], soll)) { gleich = 0; snprintf (was, sizeof was, "%s: Element %d '%s', Soll '%s'", name, i, sent[i], soll); pruef (0, was); break; }
    }
    if (gleich) pruef (1, "");
    pruef (bad_idlen == 0, "Kennungslaenge nicht 5");
    printf ("  %s: %d Zeilen, erste '%s', letzte '%s'\n", name, n_sent, n_sent ? sent[0] : "-", n_sent ? sent[(n_sent < 256 ? n_sent : 256) - 1] : "-");
}

int main (void)
{
    memset (wrap.schutz, 0xEE, sizeof wrap.schutz);
    wrap.d.ticker_deceleration = 0xDD;
    pruef (MAX_BRIGHTNESS + 1 == 16, "MAX_BRIGHTNESS + 1 ist nicht 16 -- Pruefstand nachziehen");
    tabelle ("Display", var_send_dimmed_display_colors, DISPLAY_DIMMED_DISPLAY_COLORS, wrap.d.dimmed_display_colors, 0x10);
    tabelle ("Ambilight", var_send_dimmed_ambilight_colors, DISPLAY_DIMMED_AMBILIGHT_COLORS, wrap.d.dimmed_ambilight_colors, 0x40);
    printf ("l408a: %d Pruefungen, %d FEHL -- %s\n", faelle, fehl, fehl ? "FEHL" : "OK");
    return fehl ? 1 : 0;
}
