/* Pruefstand A49 (S.19h): esp8266_idx_ok() und esp8266_cmd_reject() aus main.c. Nach vier
 * Laengenabweisungen wird eine folgende Indexabweisung TROTZDEM gemeldet; die Drosselung bleibt fuer
 * die Summe (weitere Abweisungen beider Gruende bis #49 still, #50 gemeldet). -DZIELTYPEN. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "pruefstand.h"
#ifdef ZIELTYPEN
#define uint_fast8_t uint32_t
#endif
static int logs; static char last[120];
static int log_printf (const char * f, ...) { va_list ap; va_start (ap, f); vsnprintf (last, sizeof (last), f, ap); va_end (ap); logs++; return 0; }
#include SNIPPET
static int fails, cases;
#define CHECK(name, cond) do { cases++; if (cond) printf ("  OK    %s\n", name); else { fails++; printf ("  FEHL  %s\n", name); } } while (0)
int
main (void)
{
    int i, l;
    for (i = 0; i < 4; i++) esp8266_cmd_reject ("t", 3, 9);
    CHECK ("vier Laengenabweisungen: vier Meldungen", logs == 4);
    l = logs; esp8266_idx_ok (200, 8, "night");
    CHECK ("fuenfte Abweisung, erste mit Grund Index: trotzdem gemeldet", logs == l + 1 && strstr (last, "night"));
    printf ("        (%s", last);
    l = logs;
    for (i = 0; i < 22; i++) { esp8266_idx_ok (200, 8, "night"); esp8266_cmd_reject ("t", 3, 9); }   /* #6..#49 */
    CHECK ("Abweisungen #6..#49 beider Gruende: still (Drosselung fuer die Summe)", logs == l && esp8266_cmd_reject_cnt == 49);
    l = logs; esp8266_idx_ok (200, 8, "night");
    CHECK ("Abweisung #50: gemeldet", logs == l + 1 && esp8266_cmd_reject_cnt == 50);
    printf ("\n%d Faelle, %d fehlgeschlagen\n", cases, fails);
    return fails ? 1 : 0;
}
