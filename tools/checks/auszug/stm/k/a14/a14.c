/* Pruefstand A14 (S.19f): var_send_byte/_short/_string aus src/vars/vars.c. Der Index muss als GENAU
 * zwei Hexziffern hinter der Kennung stehen, fuer jeden Wert -- 0..255 und darueber (L66, L97).
 * -DZIELTYPEN: uint_fast* 32 Bit. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pruefstand.h"
#ifdef ZIELTYPEN
#define uint_fast8_t    uint32_t
#define uint_fast16_t   uint32_t
#define uint_fast32_t   uint32_t
#endif
static char last[200];
static uint_fast8_t var_send_buf (char * b, uint_fast8_t idlen) { (void) idlen; strcpy (last, b); return 1; }
static int log_printf (const char * f, ...) { (void) f; return 0; }
#include SNIPPET
static int fails, cases;
static int
zwei (const char * id, unsigned want)                    /* genau zwei Hexziffern = (var & 0xFF), dann der Wert */
{
    char e[8];
    snprintf (e, sizeof (e), "%s%02x", id, want & 0xFF);
    return ! strncmp (last, e, strlen (e));
}
int
main (void)
{
    static const uint32_t v[] = { 0, 1, 0x30, 0xFF, 0x100, 0x1FF, 0x12345, 0xFFFFFFFFu };
    unsigned i;
    for (i = 0; i < sizeof (v) / sizeof (v[0]); i++)
    {
        int ok = 1;
        var_send_byte ("DC", v[i], 7);          ok &= zwei ("DC", v[i]) && strlen (last) == 2 + 2 + 2;
        var_send_short ("DS", v[i], 0x1234);    ok &= zwei ("DS", v[i]) && strlen (last) == 2 + 2 + 4;
        var_send_string ("S", v[i], "abc");     ok &= zwei ("S", v[i])  && strlen (last) == 1 + 2 + 3;
        cases++; if (! ok) fails++;
        printf ("  %s  var = 0x%08x -> zuletzt \"%s\"\n", ok ? "OK  " : "FEHL", (unsigned) v[i], last);
    }
    printf ("\n%d Faelle, %d fehlgeschlagen\n", cases, fails);
    return fails ? 1 : 0;
}
