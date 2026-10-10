/* Pruefstand C3 (Paket 2026-10-09, L339, AKC.2 bis AKC.5): eeprom_write() seitenweise.
 *
 * Laesst den ECHTEN Code laufen: eeprom_write() und die Seitengroesse je Ziel aus
 * src/eeprom/eeprom.c (SNIPPET, von extract.py ausgeschnitten) gegen die eingefrorene alte
 * Fassung (REF, M-Stand). Beide arbeiten auf je einem nachgebildeten AT24C32:
 *   - 4096 Byte, 16-Bit-Adresse, Adressen ueber 4095 wickeln (der Baustein ignoriert die oberen Bits);
 *   - SCHREIBEN bricht wie die Hardware am Seitenende auf den Anfang DERSELBEN Seite um
 *     (Seite des Bausteins: BAUSTEIN_P, unabhaengig von der Firmware-Konstante);
 *   - LESEN laeuft ueber Seitengrenzen und wickelt am Speicherende;
 *   - jede Transaktion und jeder Wartezyklus wird protokolliert; Lese- und Schreibfehler einspeisbar.
 *
 * Ziel per -DSTM32F4XX (Soll-Seite 32) oder -DSTM32F10X (Soll-Seite 8), Typbreiten nativ oder mit
 * -DPRUEFSTAND_SCHNELLE_TYPEN (L256). Ausgabe: FEHL-Zeilen, je Gruppe Faelle und Vergleiche, die
 * Zyklen alt/neu fuer die AKC.5-Folgen. Exit 1 bei jedem Fehlschlag.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include "pruefstand.h"
#include BEREICHE

#if defined (STM32F4XX)
#define SOLL_P      32
#elif defined (STM32F10X)
#define SOLL_P      8
#else
#error Ziel fehlt
#endif
#ifndef BAUSTEIN_P
#define BAUSTEIN_P  SOLL_P
#endif
#define MEM         4096
#define I2C_OK      0
#define TXMAX       8000

/* ---------- nachgebildetes EEPROM ---------- */
typedef struct { uint8_t m[MEM]; } IMG;
typedef struct { char t; uint32_t a; uint32_t n; int fail; } TX;
static IMG      A, B, *cur;
static TX       tx[TXMAX];
static int      ntx, nwr;
static int      rd_fail_mode;           /* 0 nie, 1 immer, 2 zufaellig jedes vierte */
static int      wr_fail_at;             /* 0 nie, sonst die K-te Schreibtransaktion */
static int      bad_addr_mode;          /* Transaktion ohne 16-Bit-Adresse */
static uint32_t frng = 0xC3C3C3C3u;

static uint32_t xs (uint32_t * s) { uint32_t x = *s; x ^= x << 13; x ^= x >> 17; x ^= x << 5; return *s = x; }

static void
txpush (char t, uint32_t a, uint32_t n, int f)
{
    if (ntx >= TXMAX) { printf ("FEHL mehr als %d Transaktionen in einem Aufruf (Endlosschleife?)\n", TXMAX); exit (1); }
    tx[ntx].t = t; tx[ntx].a = a; tx[ntx].n = n; tx[ntx].fail = f; ntx++;
}

uint_fast8_t                eeprom_is_up = 1;
static uint_fast8_t         eeprom_addr = 0xA0;

static int_fast16_t i2c_read (uint_fast8_t, uint_fast16_t, uint_fast8_t, uint8_t *, uint_fast16_t);
static int_fast16_t i2c_write (uint_fast8_t, uint_fast16_t, uint_fast8_t, uint8_t *, uint_fast16_t);
static long         n_lesen_zu_lang;    /* Lesen laenger als der Puffer der Firmware */

static void eeprom_waitstates (void) { txpush ('Z', 0, 0, 0); }

/* ---------- Umgebung der Messzeile ---------- */
static char         logbuf[256];
static int          nlog;
static uint32_t     ticks;
const uint32_t      diag_ticks_per_ms = 15;
uint32_t            diag_ticks (void) { return ticks += 7; }
static uint_fast16_t esp8266_uart_rxdrops (void) { return 0; }
static int
log_printf (const char * fmt, ...)
{
    va_list ap; va_start (ap, fmt); vsnprintf (logbuf, sizeof (logbuf), fmt, ap); va_end (ap); nlog++; return 0;
}

/* ---------- die beiden Fassungen ---------- */
#define eeprom_write ref_eeprom_write
#include REF
#undef  eeprom_write
#define eeprom_write neu_eeprom_write
#include SNIPPET
#undef  eeprom_write

/* ---------- nachgebildetes EEPROM (hinter den Auszuegen, damit EEPROM_PAGE_SIZE sichtbar ist) ---------- */
static int_fast16_t
i2c_read (uint_fast8_t sa, uint_fast16_t addr, uint_fast8_t is16, uint8_t * d, uint_fast16_t cnt)
{
    uint32_t a = ((uint32_t) addr & 0xFFFF) % MEM, i;
    int      f = rd_fail_mode == 1 || (rd_fail_mode == 2 && xs (&frng) % 4 == 0);
    (void) sa; if (is16 != 1) bad_addr_mode++;
#ifdef EEPROM_PAGE_SIZE
    if (cnt > EEPROM_PAGE_SIZE && cnt > 1) { n_lesen_zu_lang++; cnt = EEPROM_PAGE_SIZE; }   /* sonst Ueberlauf von page[] */
#endif
    txpush ('R', addr, cnt, f);
    if (f) { for (i = 0; i < cnt; i++) d[i] = (uint8_t) xs (&frng); return -1; }   /* Muell im Puffer */
    for (i = 0; i < cnt; i++) d[i] = cur->m[(a + i) % MEM];
    return I2C_OK;
}

static int_fast16_t
i2c_write (uint_fast8_t sa, uint_fast16_t addr, uint_fast8_t is16, uint8_t * d, uint_fast16_t cnt)
{
    uint32_t a = ((uint32_t) addr & 0xFFFF) % MEM, base = a - a % BAUSTEIN_P, off = a % BAUSTEIN_P, i;
    int      f;
    (void) sa; if (is16 != 1) bad_addr_mode++;
    nwr++; f = (wr_fail_at && nwr == wr_fail_at);
    txpush ('W', addr, cnt, f);
    if (f) return -1;
    for (i = 0; i < cnt; i++) cur->m[base + (off + i) % BAUSTEIN_P] = d[i];       /* Umbruch wie der Baustein */
    return I2C_OK;
}

/* ---------- Pruefung ---------- */
static long     n_fehl, n_vgl;
static int      fehl_gedruckt;
static const char * gruppe;
enum { K_SPUR, K_BAUSTEIN, K_ABBILD, K_RTC, K_ZYKLEN, K_ZYKLEN_LE, K_AKC5, K_MESS, K_AKC4, K_PUFFER, K_SONST, K_N };
static const char * k_name[K_N] = { "Spur je Seite (R/W/Z, design 2.3)", "AKC.2 Baustein-Seitengrenze", "AKC.3 Abbild",
    "AKC.3 Rueckgabewert", "Zyklen = Seiten mit Unterschied", "AKC.3 Zyklen neu <= alt, alt 0 => neu 0",
    "AKC.5 Grenzen der Folgen", "Messzeile (M.1)", "AKC.4 Fehlerpfade", "Lesen <= EEPROM_PAGE_SIZE (Puffer)", "Sonstiges" };
static long     k_vgl[K_N], k_fehl[K_N];
static int      krit = K_SONST;

static void
fehl (const char * fmt, ...)
{
    va_list ap;
    n_fehl++; k_fehl[krit]++;
    if (fehl_gedruckt++ >= 12) return;
    printf ("FEHL [%s] ", gruppe);
    va_start (ap, fmt); vprintf (fmt, ap); va_end (ap); printf ("\n");
}
#define CHECK(c, ...)  do { n_vgl++; k_vgl[krit]++; if (! (c)) fehl (__VA_ARGS__); } while (0)
#define CHECKK(k, c, ...) do { krit = (k); CHECK (c, __VA_ARGS__); krit = K_SONST; } while (0)

typedef struct { int rtc, z, nlog; char log[256]; int ntx; } RES;

static void
aufruf (int neu, uint32_t s, const uint8_t * buf, uint32_t n, RES * r)
{
    uint8_t copy[2048];
    memcpy (copy, buf, n);
    cur = neu ? &B : &A; ntx = 0; nwr = 0; nlog = 0; logbuf[0] = 0;
    r->rtc = neu ? (int) neu_eeprom_write ((uint_fast16_t) s, copy, (uint_fast16_t) n)
                 : (int) ref_eeprom_write ((uint_fast16_t) s, copy, (uint_fast16_t) n);
    r->z = 0; for (int i = 0; i < ntx; i++) if (tx[i].t == 'Z') r->z++;
    r->nlog = nlog; strcpy (r->log, logbuf); r->ntx = ntx;
    CHECK (memcmp (copy, buf, n) == 0, "%s hat den Puffer veraendert (a=%u n=%u)", neu ? "neu" : "alt", s, n);
}

/* Erwartete Spur der neuen Fassung nach design.md §2.3, aus dem Vorbild P0 errechnet. Liefert die
 * Zahl der Byte, die danach im EEPROM stehen muessen (n, oder bis zum Anfang der gescheiterten Seite). */
static long
pruef_spur (const IMG * P0, uint32_t s, const uint8_t * buf, uint32_t n, const RES * r)
{
    uint32_t pos = 0; int k = 0; int abgebrochen = 0;
    krit = K_SPUR;
    while (pos < n && ! abgebrochen)
    {
        uint32_t abs = s + pos, len = SOLL_P - abs % SOLL_P, first, end;
        if (len > n - pos) len = n - pos;
        n_vgl++; k_vgl[K_SPUR]++;
        if (k >= r->ntx || tx[k].t != 'R' || tx[k].a != abs || tx[k].n != len)
        { fehl ("Spur a=%u n=%u: erwartet R %u+%u an Stelle %d, gefunden %c %u+%u", s, n, abs, len, k,
                k < r->ntx ? tx[k].t : '-', k < r->ntx ? tx[k].a : 0, k < r->ntx ? tx[k].n : 0); krit = K_SONST; return -1; }
        first = 0; end = len;
        if (! tx[k].fail)
        {
            while (first < len && P0->m[(abs + first) % MEM] == buf[pos + first]) first++;
            while (end > first && P0->m[(abs + end - 1) % MEM] == buf[pos + end - 1]) end--;
        }
        k++;
        if (first < end)
        {
            n_vgl++; k_vgl[K_SPUR]++;
            if (k >= r->ntx || tx[k].t != 'W' || tx[k].a != abs + first || tx[k].n != end - first)
            { fehl ("Spur a=%u n=%u: erwartet W %u+%u an Stelle %d, gefunden %c %u+%u", s, n, abs + first, end - first, k,
                    k < r->ntx ? tx[k].t : '-', k < r->ntx ? tx[k].a : 0, k < r->ntx ? tx[k].n : 0); krit = K_SONST; return -1; }
            if (tx[k].fail) { k++; abgebrochen = 1; break; }
            k++;
            n_vgl++; k_vgl[K_SPUR]++;
            if (k >= r->ntx || tx[k].t != 'Z') { fehl ("Spur a=%u n=%u: nach W kein Wartezyklus an Stelle %d", s, n, k); krit = K_SONST; return -1; }
            k++;
        }
        pos += len;
    }
    if (k != r->ntx) { CHECK (0, "Spur a=%u n=%u: %d Transaktionen zu viel ab Stelle %d (%c %u+%u)", s, n, r->ntx - k, k,
           k < r->ntx ? tx[k].t : '-', k < r->ntx ? tx[k].a : 0, k < r->ntx ? tx[k].n : 0); krit = K_SONST; return -1; }
    krit = K_RTC;
    CHECK (r->rtc == (abgebrochen ? 0 : 1), "a=%u n=%u: rtc %d, erwartet %d", s, n, r->rtc, abgebrochen ? 0 : 1);
    krit = K_SONST;
    return abgebrochen ? (long) pos : (long) n;                       /* Seiten-Praefix bei Abbruch */
}

/* AKC.2 aus Sicht des Bausteins: keine Schreibtransaktion ueber seine Seitengrenze. */
static void
pruef_baustein (uint32_t s, uint32_t n, const RES * r)
{
    for (int i = 0; i < r->ntx; i++)
        if (tx[i].t == 'W')
        {
            uint32_t a = (tx[i].a & 0xFFFF) % MEM;
            CHECKK (K_BAUSTEIN, tx[i].n >= 1 && a % BAUSTEIN_P + tx[i].n <= BAUSTEIN_P,
                   "AKC.2 a=%u n=%u: Schreiben %u+%u ueber die Seitengrenze des Bausteins (%d)", s, n, a, tx[i].n, BAUSTEIN_P);
        }
}

static void
pruef_messzeile (uint32_t s, uint32_t n, const RES * r)
{
    unsigned a, nn, z;
    krit = K_MESS;
    if (r->z == 0) { CHECK (r->nlog == 0, "Messzeile ohne Zyklus (a=%u n=%u)", s, n); krit = K_SONST; return; }
    CHECK (r->nlog == 1 && sscanf (r->log, "eep a=%u n=%u z=%u", &a, &nn, &z) == 3 && a == s && nn == n && (int) z == r->z,
           "Messzeile a=%u n=%u z=%d: \"%s\" (%d Zeilen)", s, n, r->z, r->log, r->nlog);
    krit = K_SONST;
}

static void
erwartet (IMG * E, const IMG * P0, uint32_t s, const uint8_t * buf, uint32_t k)
{
    *E = *P0;
    for (uint32_t i = 0; i < k; i++) E->m[(s + i) % MEM] = buf[i];
}

/* Statistik je Gruppe */
static long g_faelle, g_zalt, g_zneu;
static long t_faelle;

/* Ein Fall ohne Fehlereinspeisung: alt und neu, alles verglichen. Liefert z alt/neu. */
static void
fall (uint32_t s, const uint8_t * buf, uint32_t n, int * zalt, int * zneu)
{
    IMG P0 = B, E; RES ra, rn; uint32_t pos; int seiten = 0;
    CHECK (memcmp (&A, &B, sizeof A) == 0, "Abbilder schon vor dem Aufruf verschieden (a=%u n=%u)", s, n);
    aufruf (0, s, buf, n, &ra);
    aufruf (1, s, buf, n, &rn);
    pruef_spur (&P0, s, buf, n, &rn);
    pruef_baustein (s, n, &rn);
    pruef_messzeile (s, n, &rn);
    pruef_messzeile (s, n, &ra);
    erwartet (&E, &P0, s, buf, n);                      /* ohne Schreibfehler: der ganze Bereich */
    CHECKK (K_ABBILD, memcmp (&B, &E, sizeof B) == 0, "AKC.3 a=%u n=%u: neues Abbild falsch", s, n);
    CHECKK (K_ABBILD, memcmp (&A, &B, sizeof A) == 0, "AKC.3 a=%u n=%u: Abbild alt != neu", s, n);
    CHECKK (K_RTC, ra.rtc == 1 && rn.rtc == 1, "AKC.3 a=%u n=%u: rtc alt %d neu %d", s, n, ra.rtc, rn.rtc);
    CHECKK (K_ZYKLEN_LE, rn.z <= ra.z, "AKC.3 a=%u n=%u: Zyklen neu %d > alt %d", s, n, rn.z, ra.z);
    CHECKK (K_ZYKLEN_LE, ra.z != 0 || rn.z == 0, "AKC.3 a=%u n=%u: alt 0 Zyklen, neu %d", s, n, rn.z);
    /* Zyklenkriterium, unabhaengig von der Spur: genau ein Zyklus je Soll-Seite mit Unterschied */
    for (pos = 0; pos < n; )
    {
        uint32_t len = SOLL_P - (s + pos) % SOLL_P, d = 0;
        if (len > n - pos) len = n - pos;
        for (uint32_t i = 0; i < len; i++) d |= P0.m[(s + pos + i) % MEM] != buf[pos + i];
        seiten += rd_fail_mode == 1 ? 1 : (int) d;      /* Lesefehler: jede Seite gilt als verschieden */
        pos += len;
    }
    CHECKK (K_ZYKLEN, rn.z == seiten, "Zyklen a=%u n=%u: %d, erwartet %d (Seiten mit Unterschied)", s, n, rn.z, seiten);
    if (memcmp (&A, &B, sizeof A) != 0) A = B;          /* Folgefaelle nicht mit verschleppen */
    g_faelle++; t_faelle++; g_zalt += ra.z; g_zneu += rn.z;
    if (zalt) *zalt = ra.z;
    if (zneu) *zneu = rn.z;
}

static void
gruppe_start (const char * name) { gruppe = name; g_faelle = g_zalt = g_zneu = 0; }
static long vgl_vorher;
static void
gruppe_ende (void)
{
    printf ("  %-40s Faelle %6ld  Zyklen alt %7ld neu %6ld\n", gruppe, g_faelle, g_zalt, g_zneu);
}

/* Inhalt erzeugen: Art 0 unveraendert, 1 ein Byte, 2 duenn (1/16), 3 dicht (1/2), 4 alles neu */
static uint32_t rng = 0x10C3u;
static void
inhalt (uint8_t * buf, uint32_t s, uint32_t n, int art)
{
    for (uint32_t i = 0; i < n; i++)
    {
        uint8_t alt = B.m[(s + i) % MEM];
        int neu_byte = art == 4 || (art == 3 && xs (&rng) % 2) || (art == 2 && xs (&rng) % 16 == 0);
        buf[i] = neu_byte ? (uint8_t) (alt ^ (1 + xs (&rng) % 255)) : alt;
    }
    if (art == 1 && n) { uint32_t i = xs (&rng) % n; buf[i] = (uint8_t) (B.m[(s + i) % MEM] ^ 0x5A); }
}

static void
setze_text (uint8_t * buf, uint32_t size, const char * s)
{
    memset (buf, 0, size); strncpy ((char *) buf, s, size - 1);       /* wie write_update_host_to_eep() */
}

int
main (void)
{
    uint8_t buf[2048];                                      /* groesster Bereich: Overlays, 1280 Byte */
    int     zalt, zneu, zmax;

    printf ("Pruefstand C3, Soll-Seite %d, Baustein-Seite %d, uint_fast16_t %d Bit\n", SOLL_P, BAUSTEIN_P, (int) (8 * sizeof (uint_fast16_t)));
    for (int i = 0; i < MEM; i++) B.m[i] = (uint8_t) xs (&rng);
    A = B;

    /* Plausibilitaet der Bereiche: lueckenlos ab 0, innerhalb 4096 */
    gruppe = "Bereiche";
    for (int i = 0; i < N_BEREICHE; i++)
        CHECK (i == 0 ? bereiche[i].off == 0 : bereiche[i].off == bereiche[i - 1].off + bereiche[i - 1].size,
               "Bereich %s nicht lueckenlos", bereiche[i].name);
    CHECK (EEPROM_DATA_END <= MEM && EEPROM_DATA_END == bereiche[N_BEREICHE - 1].off + bereiche[N_BEREICHE - 1].size, "EEPROM_DATA_END");

    /* 1. Jede Startadresse, Laengen um die Seitengrenze */
    gruppe_start ("jede Startadresse x 6 Laengen");
    {
        uint32_t lens[6] = { 1, SOLL_P - 1, SOLL_P, SOLL_P + 1, 2 * SOLL_P + 3, 600 };
        for (uint32_t s = 0; s < MEM; s++)
            for (int j = 0; j < 6; j++) { inhalt (buf, s, lens[j], 4); fall (s, buf, lens[j], 0, 0); }
    }
    gruppe_ende ();

    /* 2. Jeder Bereich aus eeprom-data.h: voll, Teil, ein Byte (Anfang/Mitte/Ende), unveraendert */
    gruppe_start ("Bereiche aus eeprom-data.h (x 7)");
    for (int i = 0; i < N_BEREICHE; i++)
    {
        uint32_t o = bereiche[i].off, z = bereiche[i].size;
        inhalt (buf, o, z, 4); fall (o, buf, z, 0, 0);                                  /* voll neu */
        inhalt (buf, o, z, 2); fall (o, buf, z, 0, 0);                                  /* duenn */
        if (z >= 4) { inhalt (buf, o + z / 4, z / 2, 4); fall (o + z / 4, buf, z / 2, 0, 0); }   /* Teil */
        else { inhalt (buf, o, z, 3); fall (o, buf, z, 0, 0); }
        inhalt (buf, o, 1, 4); fall (o, buf, 1, 0, 0);                                  /* erstes Byte */
        inhalt (buf, o + z / 2, 1, 4); fall (o + z / 2, buf, 1, 0, 0);                  /* mittleres */
        inhalt (buf, o + z - 1, 1, 4); fall (o + z - 1, buf, 1, 0, 0);                  /* letztes */
        inhalt (buf, o, z, 0); fall (o, buf, z, 0, 0);                                  /* unveraendert */
    }
    for (int i = 0; i < MAX_OVERLAYS; i++)                                              /* jeder Overlay-Eintrag */
    {
        uint32_t o = EEPROM_DATA_OFFSET_OVERLAY + i * OVERLAY_ENTRY_SIZE;
        inhalt (buf, o, OVERLAY_ENTRY_SIZE, 3); fall (o, buf, OVERLAY_ENTRY_SIZE, 0, 0);
    }
    gruppe_ende ();

    /* 3. Zufallsfolge ueber den ganzen Adressraum, Laengen 0..600, fester Startwert */
    gruppe_start ("Zufall 0..4095, Laenge 0..600");
    for (int c = 0; c < 12000; c++)
    {
        uint32_t s = xs (&rng) % MEM, n = xs (&rng) % 601;
        inhalt (buf, s, n, (int) (xs (&rng) % 5));
        fall (s, buf, n, 0, 0);
    }
    gruppe_ende ();

    /* 4. AKC.4 Fehlerpfade */
    gruppe_start ("AKC.4 Lesefehler immer");
    rd_fail_mode = 1;
    for (int c = 0; c < 1000; c++)
    {
        uint32_t s = xs (&rng) % MEM, n = 1 + xs (&rng) % 200;
        inhalt (buf, s, n, (int) (xs (&rng) % 5)); fall (s, buf, n, 0, 0);
    }
    gruppe_ende ();
    gruppe_start ("AKC.4 Lesefehler zufaellig (1/4)");
    rd_fail_mode = 2;
    for (int c = 0; c < 2000; c++)
    {
        IMG P0 = B, E; RES rn; uint32_t s = xs (&rng) % MEM, n = 1 + xs (&rng) % 200; long k;
        inhalt (buf, s, n, (int) (xs (&rng) % 5));
        aufruf (1, s, buf, n, &rn);
        k = pruef_spur (&P0, s, buf, n, &rn); pruef_baustein (s, n, &rn); pruef_messzeile (s, n, &rn);
        (void) k;
        erwartet (&E, &P0, s, buf, n);
        CHECKK (K_AKC4, memcmp (&B, &E, sizeof B) == 0 && rn.rtc == 1, "AKC.4 Lesefehler a=%u n=%u: Abbild falsch oder rtc %d", s, n, rn.rtc);
        A = B; g_faelle++; t_faelle++; g_zneu += rn.z;
    }
    rd_fail_mode = 0;
    gruppe_ende ();

    gruppe_start ("AKC.4 Schreibfehler an der K-ten Seite");
    for (int c = 0; c < 2000; c++)
    {
        IMG SA = A, SB = B, P0 = B, E; RES ra, rn; uint32_t s = xs (&rng) % MEM, n = 1 + xs (&rng) % 200, pos = 0; long k;
        int seiten = 0, kk;
        inhalt (buf, s, n, (int) (xs (&rng) % 4) + 1);
        /* Seiten mit Unterschied zaehlen, um K sinnvoll zu waehlen */
        for (pos = 0; pos < n; ) { uint32_t len = SOLL_P - (s + pos) % SOLL_P, d = 0; if (len > n - pos) len = n - pos;
            for (uint32_t i = 0; i < len; i++) d |= P0.m[(s + pos + i) % MEM] != buf[pos + i]; seiten += d; pos += len; }
        if (! seiten) continue;
        kk = 1 + (int) (xs (&rng) % (uint32_t) seiten);
        wr_fail_at = kk;
        aufruf (1, s, buf, n, &rn);
        k = pruef_spur (&P0, s, buf, n, &rn);
        krit = K_AKC4;
        CHECK (rn.rtc == 0, "AKC.4 Schreibfehler a=%u n=%u K=%d: rtc %d", s, n, kk, rn.rtc);
        CHECK (ntx > 0 && tx[ntx - 1].t == 'W' && tx[ntx - 1].fail, "AKC.4 a=%u n=%u K=%d: Verkehr nach dem Schreibfehler", s, n, kk);
        /* Seiten-Praefix: alles vor der gescheiterten Seite geschrieben, ab dort nichts */
        if (k >= 0)                                         /* weicht die Spur ab, meldet das schon K_SPUR */
        {
            erwartet (&E, &P0, s, buf, (uint32_t) k);
            CHECK (k < (long) n, "AKC.4 a=%u n=%u K=%d: Abbruch nicht erkannt", s, n, kk);
            CHECK (memcmp (&B, &E, sizeof B) == 0, "AKC.4 a=%u n=%u K=%d: kein Seiten-Praefix", s, n, kk);
        }
        wr_fail_at = 1;                                     /* alt: erste Schreibtransaktion scheitert */
        aufruf (0, s, buf, n, &ra);
        CHECK (ra.rtc == 0, "AKC.4 alt a=%u n=%u: rtc %d", s, n, ra.rtc);
        wr_fail_at = 0; krit = K_SONST;
        A = SA; B = SB;
        g_faelle++; t_faelle++;
    }
    gruppe_ende ();

    gruppe_start ("AKC.4 cnt == 0, eeprom_is_up == 0");
    for (uint32_t s = 0; s < MEM; s += 97)
    {
        RES ra, rn;
        aufruf (0, s, buf, 0, &ra); aufruf (1, s, buf, 0, &rn);
        krit = K_AKC4;
        CHECK (ra.rtc == 1 && rn.rtc == 1 && ra.ntx == 0 && rn.ntx == 0 && rn.nlog == 0, "cnt==0 a=%u: rtc %d/%d, %d/%d Transaktionen", s, ra.rtc, rn.rtc, ra.ntx, rn.ntx);
        eeprom_is_up = 0;
        inhalt (buf, s, 40, 4);
        aufruf (0, s, buf, 40, &ra); aufruf (1, s, buf, 40, &rn);
        CHECK (ra.rtc == 0 && rn.rtc == 0 && ra.ntx == 0 && rn.ntx == 0 && rn.nlog == 0, "is_up==0 a=%u: rtc %d/%d, %d/%d Transaktionen", s, ra.rtc, rn.rtc, ra.ntx, rn.ntx);
        eeprom_is_up = 1;
        CHECK (memcmp (&A, &B, sizeof A) == 0, "is_up==0 hat geschrieben");
        krit = K_SONST;
        g_faelle += 2; t_faelle += 2;
    }
    gruppe_ende ();

    /* 5. AKC.5: die beiden Folgen aus G2 / S.26 */
    {
        static const char * lang = "update-server.mit.einem.sehr.langen.namen.beispiel.example.com/"; /* 63 Zeichen */
        int grenze_s26 = SOLL_P == 32 ? 3 : 9, grenze_wo = SOLL_P == 32 ? 2 : 5;
        uint32_t ho = EEPROM_DATA_OFFSET_UPDATE_HOSTNAME, hz = EEPROM_DATA_SIZE_UPDATE_HOSTNAME;
        uint32_t po = EEPROM_DATA_OFFSET_UPDATE_PATH, pz = EEPROM_DATA_SIZE_UPDATE_PATH;
        uint32_t wo = EEPROM_DATA_OFFSET_WEATHER_CITY, wz = EEPROM_DATA_SIZE_WEATHER_CITY;
        static const char * orte[4] = { "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB", "Zuerich", "Bern" };
        CHECK (strlen (lang) == 63, "Folge S.26: Testtext nicht 63 Zeichen");

        gruppe_start ("AKC.5 S.26 Host/Pfad 63 <-> kurz");
        zmax = 0;
        for (int r = 0; r < 6; r++)
        {
            setze_text (buf, hz, r % 2 ? "u.ch" : lang); fall (ho, buf, hz, &zalt, &zneu); if (zneu > zmax) zmax = zneu;
            printf ("    Host a=%u n=%u %-5s z alt %2d neu %d\n", ho, hz, r % 2 ? "kurz" : "lang", zalt, zneu);
            setze_text (buf, pz, r % 2 ? "/x" : lang); fall (po, buf, pz, &zalt, &zneu); if (zneu > zmax) zmax = zneu;
            printf ("    Pfad a=%u n=%u %-5s z alt %2d neu %d\n", po, pz, r % 2 ? "kurz" : "lang", zalt, zneu);
        }
        CHECKK (K_AKC5, zmax <= grenze_s26, "AKC.5 S.26: neu hoechstens %d Zyklen erwartet, gemessen %d", grenze_s26, zmax);
        gruppe_ende ();

        gruppe_start ("AKC.5 Wetterort (G2-Folge)");
        zmax = 0;
        for (int r = 0; r < 10; r++)
        {
            const char * o = orte[r < 6 ? r % 2 : 2 + r % 2];
            memset (buf, 0, wz); strncpy ((char *) buf, o, wz);                          /* wie weather_set_city() */
            fall (wo, buf, wz, &zalt, &zneu); if (zneu > zmax) zmax = zneu;
            printf ("    Ort a=%u n=%u %-8.8s z alt %2d neu %d\n", wo, wz, o, zalt, zneu);
        }
        CHECKK (K_AKC5, zmax <= grenze_wo, "AKC.5 Wetterort: neu hoechstens %d Zyklen erwartet, gemessen %d", grenze_wo, zmax);
        gruppe_ende ();
    }

    CHECKK (K_PUFFER, n_lesen_zu_lang == 0, "%ld Lesezugriffe laenger als EEPROM_PAGE_SIZE (Pufferueberlauf)", n_lesen_zu_lang);
    CHECK (bad_addr_mode == 0, "%d Transaktionen ohne 16-Bit-Adresse", bad_addr_mode);
    (void) vgl_vorher;
    printf ("  Kriterien (Vergleiche / FEHL):\n");
    for (int i = 0; i < K_N; i++)
        printf ("    %-45s %8ld / %ld%s\n", k_name[i], k_vgl[i], k_fehl[i], k_fehl[i] ? "   <- FEHL" : "");
    printf ("Faelle %ld, Vergleiche %ld, Bereiche %d, Soll-Seite %d: %s\n", t_faelle, n_vgl, N_BEREICHE, SOLL_P,
            n_fehl ? "FEHL" : "OK");
    if (n_fehl) printf ("FEHL insgesamt %ld Befunde\n", n_fehl);
    return n_fehl ? 1 : 0;
}
