/* Pruefstand N5 (Paket 2026-10-11, S3.2): snprintf/sprintf als Huellen um vsnprintf (stubs.c).
 * Vergleicht die Huellen (woertlich, umbenannt) mit der libc des Hosts ueber JEDES Format, das die Quellen unter src
 * an sprintf/snprintf uebergibt, mit Rand- und Zufallswerten: Rueckgabewert, Pufferinhalt bis zum Abschluss
 * und die Bytes dahinter (Waechter 0xA5). snprintf mit size 0, 1, 3, 8, 64.
 * Was der Host NICHT zeigt: newlib selbst. Dass newlibs snprintf == vsnprintf und sprintf ==
 * vsnprintf (INT_MAX) ist, ist am Maschinencode von libc_nano.a belegt (Bericht, Kommentar in stubs.c). */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <stddef.h>
#include "huellen.c"
static unsigned long faelle, fehl;
static char a[256], b[256];
static void vergl (const char * art, const char * fmt, int ra, int rb, size_t n)
{
    faelle++;
    if (ra != rb || memcmp (a, b, n))
    {
        fehl++;
        if (fehl <= 6) printf ("  FEHL  %s \"%s\": libc %d \"%.60s\", Huelle %d \"%.60s\"\n", art, fmt, ra, a, rb, b);
    }
}
#define VERGL_SPRINTF(f, ...) do { memset (a, 0xA5, sizeof a); memset (b, 0xA5, sizeof b); \
    int ra = sprintf (a, f, ##__VA_ARGS__); int rb = w_sprintf (b, f, ##__VA_ARGS__); vergl ("sprintf", f, ra, rb, sizeof a); } while (0)
#define VERGL_SNPRINTF(s, f, ...) do { memset (a, 0xA5, sizeof a); memset (b, 0xA5, sizeof b); \
    int ra = snprintf (a, s, f, ##__VA_ARGS__); int rb = w_snprintf (b, s, f, ##__VA_ARGS__); vergl ("snprintf", f, ra, rb, sizeof a); } while (0)
#include "faelle.c"
int main (void)
{
    alle_faelle ();
    printf ("  geprueft: %s; Formate aus src: %d, Vergleiche: %lu (Soll %d)\n", FASSUNG, N_FORMATE, faelle, N_VERGLEICHE);
    if (faelle != N_VERGLEICHE) { printf ("  FEHL  Zaehlung\n"); fehl++; }
    printf ("n5: %lu Pruefungen, %lu FEHL -- %s\n", faelle, fehl, fehl ? "FEHL" : "OK");
    return fehl ? 1 : 0;
}
