/* Pruefstand GT (Paket 2026-10-10, S0): my_gmtime() alt (64-Bit-time_t intern) gegen neu
 * (uint32_t intern), alle Felder von struct tm, Eingabe 0..2^32-1. Zusaetzlich Referenz:
 * gmtime_r() der Host-libc (64-Bit-time_t) -- berichtet, kein Massstab. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#ifdef PRUEFSTAND_SCHNELLE_TYPEN
#include "pruefstand.h"
#endif
struct tm * gm_alt (time_t *);
struct tm * gm_neu (time_t *);
typedef char time_t_64[(sizeof (time_t) == 8) ? 1 : -1];

static unsigned long faelle, fehl, ref_alt, ref_neu, gemeldet;
static unsigned long fehl_kat[8];

static int gleich (const struct tm * a, const struct tm * b)
{
    return a->tm_sec == b->tm_sec && a->tm_min == b->tm_min && a->tm_hour == b->tm_hour
        && a->tm_mday == b->tm_mday && a->tm_mon == b->tm_mon && a->tm_year == b->tm_year
        && a->tm_wday == b->tm_wday && a->tm_yday == b->tm_yday && a->tm_isdst == b->tm_isdst;
}
static void zeig (const char * w, const struct tm * x)
{
    printf ("      %-4s %04d-%02d-%02d %02d:%02d:%02d wday=%d yday=%d isdst=%d\n", w, x->tm_year + 1900,
            x->tm_mon + 1, x->tm_mday, x->tm_hour, x->tm_min, x->tm_sec, x->tm_wday, x->tm_yday, x->tm_isdst);
}
static void pruef (uint32_t v, int kat)
{
    time_t ta = (time_t) v, tn = (time_t) v, tr = (time_t) v;
    struct tm a, n, r;
    a = *gm_alt (&ta);
    n = *gm_neu (&tn);
    gmtime_r (&tr, &r);
    faelle++;
    if (! gleich (&a, &n))
    {
        fehl++; fehl_kat[kat]++;
        if (gemeldet++ < 5) { printf ("  FEHL  alt != neu bei %lu (Kategorie %d)\n", (unsigned long) v, kat); zeig ("alt", &a); zeig ("neu", &n); }
    }
    if (! gleich (&a, &r)) { if (ref_alt++ < 3) { printf ("  BEFUND alt != gmtime_r bei %lu\n", (unsigned long) v); zeig ("alt", &a); zeig ("ref", &r); } }
    if (! gleich (&n, &r)) ref_neu++;
}
static uint32_t x = 2463534242u;
static uint32_t zufall (void) { x ^= x << 13; x ^= x >> 17; x ^= x << 5; return x; }
static uint32_t ts (int y, int m, int d, int hh, int mm, int ss)   /* unabhaengig: Tage nach Hinnant */
{
    int64_t yy = y - (m <= 2); int64_t era = (yy >= 0 ? yy : yy - 399) / 400; int64_t yoe = yy - era * 400;
    int64_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1; int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return (uint32_t) ((era * 146097 + doe - 719468) * 86400 + hh * 3600 + mm * 60 + ss);
}
int main (void)
{
    static const uint32_t sek[4] = { 0, 1, 43200, 86399 };
    unsigned long k[8] = { 0 }, vor;
    uint64_t d, s;
    int y, m, i, j;

    /* 0: jeder Tag 1970..2106 mit den Sekunden 0, 1, 43200, 86399 */
    vor = faelle;
    for (d = 0; d * 86400 <= 0xFFFFFFFFu; d++)
        for (i = 0; i < 4; i++) { s = d * 86400 + sek[i]; if (s <= 0xFFFFFFFFu) pruef ((uint32_t) s, 0); }
    k[0] = faelle - vor;

    /* 1: jeder Monatswechsel 1970-02..2106-02, jede Sekunde von -120 bis +120 */
    vor = faelle;
    for (y = 1970; y <= 2106; y++)
        for (m = 1; m <= 12; m++)
        {
            int64_t b = (int64_t) ts (y, m, 1, 0, 0, 0);
            if (y == 2106 && m > 2) break;
            for (j = -120; j <= 120; j++) { int64_t v = b + j; if (v >= 0 && v <= 0xFFFFFFFFLL) pruef ((uint32_t) v, 1); }
        }
    k[1] = faelle - vor;

    /* 2: benannte Grenzen */
    vor = faelle;
    {
        uint32_t g[] = {
            0, 1, 59, 60, 3599, 3600, 86399, 86400,
            ts (2000, 2, 28, 23, 59, 59), ts (2000, 2, 29, 0, 0, 0), ts (2000, 2, 29, 12, 0, 0), ts (2000, 2, 29, 23, 59, 59), ts (2000, 3, 1, 0, 0, 0),
            ts (2000, 12, 31, 23, 59, 59), ts (2001, 1, 1, 0, 0, 0),
            ts (2038, 1, 19, 3, 14, 7), ts (2038, 1, 19, 3, 14, 8), 0x7FFFFFFFu, 0x80000000u,
            ts (2100, 2, 28, 0, 0, 0), ts (2100, 2, 28, 23, 59, 59), ts (2100, 3, 1, 0, 0, 0), ts (2100, 3, 1, 12, 0, 0),
            ts (2100, 12, 31, 23, 59, 59), ts (2101, 1, 1, 0, 0, 0),
            ts (2106, 2, 7, 6, 28, 14), 0xFFFFFFFEu, 0xFFFFFFFFu
        };
        if (ts (2038, 1, 19, 3, 14, 7) != 0x7FFFFFFFu || ts (2106, 2, 7, 6, 28, 15) != 0xFFFFFFFFu) { printf ("  FEHL  Referenzrechnung ts()\n"); fehl++; }
        for (i = 0; i < (int) (sizeof g / sizeof g[0]); i++) pruef (g[i], 2);
        /* ausdruecklich: 2100-03-01 muss Maerz sein, kein 29. Februar */
        { time_t t = ts (2100, 3, 1, 0, 0, 0); struct tm * n = gm_neu (&t); faelle++;
          if (! (n->tm_year == 200 && n->tm_mon == 2 && n->tm_mday == 1 && n->tm_wday == 1 && n->tm_yday == 59)) { fehl++; fehl_kat[2]++; printf ("  FEHL  2100-03-01 nicht Montag, 1. Maerz, yday 59\n"); zeig ("neu", n); } }
        { time_t t = 0xFFFFFFFFu; struct tm * n = gm_neu (&t); faelle++;
          if (! (n->tm_year == 206 && n->tm_mon == 1 && n->tm_mday == 7 && n->tm_hour == 6 && n->tm_min == 28 && n->tm_sec == 15 && n->tm_wday == 0)) { fehl++; fehl_kat[2]++; printf ("  FEHL  0xFFFFFFFF nicht So 2106-02-07 06:28:15\n"); zeig ("neu", n); } }
    }
    k[2] = faelle - vor;

    /* 3: 100'000 Zufallswerte, fester Startwert */
    vor = faelle;
    for (i = 0; i < 100000; i++) pruef (zufall (), 3);
    k[3] = faelle - vor;

    printf ("  Faelle: Tage %lu, Monatswechsel %lu, Grenzen %lu, Zufall %lu, gesamt %lu\n", k[0], k[1], k[2], k[3], faelle);
    printf ("  Abweichungen alt != neu: %lu (Tage %lu, Monatswechsel %lu, Grenzen %lu, Zufall %lu)\n", fehl, fehl_kat[0], fehl_kat[1], fehl_kat[2], fehl_kat[3]);
    printf ("  Referenz gmtime_r: alt weicht ab in %lu, neu in %lu Faellen\n", ref_alt, ref_neu);
    if (faelle < 300000) { printf ("  FEHL  weniger als 300'000 Faelle\n"); return 1; }
    printf (fehl ? "  FEHL  GT\n" : "  OK    GT\n");
    return fehl ? 1 : 0;
}
