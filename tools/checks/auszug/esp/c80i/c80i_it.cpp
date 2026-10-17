/* c80i, Fall 6 it: Der Waechter it ist bei intakten nb/ord nicht erreichbar (Folge streng steigend und < Blockzahl).
 * Er ist der Rueckhalt, falls ord je wegfaellt. Diese Probe uebersetzt den reinen Abschnitt OHNE ord (rein_ohne_ord.inc)
 * und zeigt an einem Zyklus, dass der Gang dann mit abbruch=it endet statt endlos zu laufen. alarm (2) faengt den Fall,
 * dass auch it fehlt. Exit 0 OK, 1 FEHL.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
#define PROGMEM
#define PSTR(s) (s)
typedef const char * PGM_P;
#define snprintf_P snprintf
#define vsnprintf_P vsnprintf
#define memcpy_P memcpy
#include "heapkarte.h"
#include "rein_ohne_ord.inc"
int main (void)
{
    uint8_t m[8 * 100] = { 0 };
    uint16_t v;
    HK_KARTE k;
    alarm (2);
    v = 1;  memcpy (&m[0], &v, 2);                                          /* 1 -> 11 frei -> 31 -> 11 (Zyklus) */
    v = 11; memcpy (&m[8], &v, 2);
    v = 31 | 0x8000; memcpy (&m[88], &v, 2);
    v = 11; memcpy (&m[248], &v, 2);
    uint8_t ab = hk_gang (m, 100, 8, &k);
    if (ab != HK_AB_IT) { printf ("FEHLER Fall 6 it: ohne ord endet der Zyklus mit abbruch=%u, nicht it (%u)\n", ab, HK_AB_IT); return 1; }
    printf ("  Fall 6 it: Zyklus ohne ord -> abbruch=it\n");
    return 0;
}
