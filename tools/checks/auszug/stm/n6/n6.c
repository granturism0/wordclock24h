/* Pruefstand N6 (Paket 2026-10-11, S3.2): Praefix "DFPlayer: " einmal statt 48-mal.
 *
 * Uebersetzt je Seite die ECHTE log.c/log.h und JEDEN Logaufruf aus dfplayer.c (gesammelt, nachgezaehlt) und
 * schreibt mit, was ankommt: U[...] = Bytes an die Debug-UART (log_uart_puts), E[...] = eine Zeile an den
 * ESP-Ring (esp8266_send_log_line). Seite ALT = eingefrorener Stand 3.2.30 (alt-*.c, alt-log.h), Seite NEU =
 * aktueller Stand. run.sh vergleicht die beiden Mitschriften Byte fuer Byte.
 * Dazu Laengen 230..262 Zeichen hinter dem Praefix: das Abschneiden an STRBUF_SIZE 256 muss gleich bleiben. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

/* UART: ein Bytestrom (sie kennt keine Zeilen je Aufruf), ESP-Ring: je Aufruf von esp8266_send_log_line
 * EINE Zeile E[..]. Die Mitschrift je Logaufruf ist "U[alle UART-Bytes] E[..]E[..]". */
static char uart[1 << 15], esp[1 << 15], mitschrift[1 << 16]; static size_t upos, epos, mpos;
static void esc (char * ziel, size_t * pos, size_t max, const char * s)
{
    for (; *s; s++)
        *pos += snprintf (ziel + *pos, max - *pos, (*s >= 32 && *s < 127 && *s != '\\') ? "%c" : "\\x%02x", (unsigned char) *s);
}
static void schreib (const char * art, const char * s)
{
    if (art[0] == 'U') esc (uart, &upos, sizeof uart, s);
    else { epos += snprintf (esp + epos, sizeof esp - epos, "E["); esc (esp, &epos, sizeof esp, s); epos += snprintf (esp + epos, sizeof esp - epos, "]"); }
    snprintf (mitschrift, sizeof mitschrift, "U[%s] %s", uart, esp); mpos = 1;
}
#define NEU() do { upos = epos = 0; uart[0] = esp[0] = 0; mitschrift[0] = 0; } while (0)
void log_uart_puts (const char * s) { schreib ("U", s); }
void log_uart_putc (int c) { char b[2] = { (char) c, 0 }; schreib ("U", b); }
void log_uart_init (uint32_t b) { (void) b; }
void log_uart_flush (void) { }
void esp8266_send_log_line (const char * s) { schreib ("E", s); }

#include "log.h"
#include "log.c"
#include "helfer.c"

/* Variablen, die die Logaufrufe in dfplayer.c benutzen */
#define PAR1_BYTE_POS 5
#define PAR2_BYTE_POS 6
static uint8_t  buf[16];
static int      par, file, volume, eq, device;

static void aufruf (int i)
{
    switch (i)
    {
#include "aufrufe.c"
    }
}

int main (void)
{
    static const int werte[][7] = { { 0, 0, 0, 0, 0, 0, 0 }, { 1, 1, 1, 1, 1, 1, 1 }, { 7, 0x12, 0x34, 3, 30, 5, 4 },
                                    { 255, 0xFF, 0x00, 255, 31, 6, 2 }, { 65535, 0xAB, 0xCD, 65535, 0, 0, 1 },
                                    { -1, 0x7E, 0xEF, -1, -1, -1, -1 }, { 2147483647, 0x01, 0x02, -2147483647 - 1, 15, 2, 3 } };
    int v, i, L;
    char lang[400], msg[420];

    for (v = 0; v < (int) (sizeof werte / sizeof werte[0]); v++)
    {
        par = werte[v][0]; buf[PAR1_BYTE_POS] = (uint8_t) werte[v][1]; buf[PAR2_BYTE_POS] = (uint8_t) werte[v][2];
        file = werte[v][3]; volume = werte[v][4]; eq = werte[v][5]; device = werte[v][6];
        for (i = 0; i < N_AUFRUFE; i++)
        {
            NEU (); aufruf (i);
            printf ("W%d A%02d %s\n", v, i, mitschrift);
        }
    }
    for (L = 230; L <= 262; L++)
    {
        memset (lang, 0, sizeof lang);
        for (i = 0; i < L; i++) lang[i] = (char) ('a' + i % 26);
#ifdef SEITE_ALT
        NEU (); log_printf ("DFPlayer: %s\r\n", lang);              printf ("L%d printf+crlf %s\n", L, mitschrift);
        NEU (); log_printf ("DFPlayer: %s", lang);                  printf ("L%d printf %s\n", L, mitschrift);
        snprintf (msg, sizeof msg, "DFPlayer: %s", lang);
        NEU (); log_message (msg);                                 printf ("L%d message %s\n", L, mitschrift);
#else
        NEU (); dfp_printf ("%s\r\n", lang);                        printf ("L%d printf+crlf %s\n", L, mitschrift);
        NEU (); dfp_printf ("%s", lang);                            printf ("L%d printf %s\n", L, mitschrift);
        (void) msg; NEU (); dfp_message (lang);                    printf ("L%d message %s\n", L, mitschrift);
#endif
    }
    printf ("N_AUFRUFE %d\n", N_AUFRUFE);
    return 0;
}
