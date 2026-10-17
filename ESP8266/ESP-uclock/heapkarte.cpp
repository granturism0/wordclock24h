/*----------------------------------------------------------------------------------------------------------------------------------------
 * heapkarte.cpp - befristetes Messinstrument zu C80 (specs/paket-2026-10-17-c80, design.md 4 N1/N2). Zweck, Kosten und
 * Rueckbaupflicht: heapkarte.h.
 *
 * Aufbau:
 *   Abschnitt "C80I REIN" - reine Funktionen ohne Geraet: Blockgang, Toepfe, Ereignisse, Takt, Zeilenformen. Der
 *                           Pruefstand c80i schneidet ihn aus und uebersetzt ihn auf dem Entwicklungsrechner.
 *   Geraeteteil           - Sperre um den Blockgang, Lesen der lwIP-Listen und der Wegmarken, Ausgabe auf beiden Wegen.
 *
 * Kein Zugriff legt etwas auf den Haufen: keine String, kein new/malloc, Zeilen per snprintf_P in Stackpuffer.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#include "heapkarte.h"

#if ESP_HEAP_MAP

#include <Arduino.h>
#include <core_version.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <stddef.h>
#include <stdarg.h>
#include <lwip/tcp.h>
#include <lwip/priv/tcp_priv.h>                                         // nur lesend: tcp_active_pcbs, tcp_tw_pcbs, Listen-PCBs.
                                                                        // Ent-32 (Paket 2026-10-13) sieht den Kopf nur in
                                                                        // http.cpp vor; die Spec C80 legt das Lesen hierher
                                                                        // (design.md 9 N2). Befristet wie das Instrument.
#include <lwip/udp.h>
#include <lwip/netif.h>
#include <lwip/dhcp.h>
#include "base.h"
#include <umm_malloc/umm_malloc.h>
#include <umm_malloc/umm_local.h>                                           // ZULETZT: definiert memcpy/memmove/memset um
#undef memcpy
#undef memmove
#undef memset

/* Bindung an Core 3.1.2 (Ent-C1 = A, design.md 4.7 N1): Die Blockform unten ist aus umm_malloc.cpp:104-120 dieses Cores
 * nachgebildet, der Kontext aus umm_local.h:64-75. Ein anderer Core oder eine andere Heap-Konfiguration bricht HIER ab,
 * statt still falsche Karten zu liefern.
 */
#if ! defined (ARDUINO_ESP8266_RELEASE_3_1_2)
#error "heapkarte.cpp (C80): nur fuer ESP8266-Core 3.1.2 - Blockform und Kontext pruefen oder ESP_HEAP_MAP 0"
#endif

#if UMM_NUM_HEAPS != 1
#error "heapkarte.cpp (C80): genau ein Haufen erwartet (UMM_NUM_HEAPS)"
#endif

#if defined (UMM_POISON_CHECK) || defined (UMM_POISON_CHECK_LITE) || defined (UMM_INTEGRITY_CHECK)
#error "heapkarte.cpp (C80): Heap-Pruefoptionen aendern Belegungsgroessen - Karte nicht vergleichbar"
#endif

static_assert (sizeof (umm_heap_context_t) == 52, "umm_heap_context_t: Groesse wie Core 3.1.2");
static_assert (offsetof (umm_heap_context_t, heap) == 0, "umm_heap_context_t: heap am Anfang");
static_assert (offsetof (umm_heap_context_t, numblocks) == 48, "umm_heap_context_t: numblocks an Versatz 48");

#endif // ESP_HEAP_MAP (Kopf)

#if ESP_HEAP_MAP
/*== C80I REIN ANFANG ===================================================================================================================*/

/* Blockform umm_malloc, Core 3.1.2 (umm_malloc.cpp:104-120): je Block next/prev zu 16 Bit, dann 4 Byte Nutzlast bzw.
 * die Verkettung der Freiliste. In next ist Bit 15 das Frei-Bit, Bits 0-14 der Index des Folgeblocks. Block 0 ist der
 * Kopf der Freiliste, der letzte Block der Endwaechter mit next == 0. Benachbarte freie Eintraege gibt es nicht
 * (umm_free () legt in beide Richtungen zusammen, umm_malloc.cpp:638-646).
 */
typedef struct
{
    uint16_t        next;
    uint16_t        prev;
    uint8_t         body[4];
} __attribute__ ((packed)) HK_BLOCK;

static_assert (sizeof (HK_BLOCK) == 8, "HK_BLOCK");

#define HK_FREI_BIT                 0x8000                                  // UMM_FREELIST_MASK
#define HK_INDEX_MASKE              0x7FFF                                  // UMM_BLOCKNO_MASK

static uint16_t
hk_ms16 (uint32_t ms)
{
    return ms > 0xFFFFUL ? (uint16_t) 0xFFFF : (uint16_t) ms;
}

static void
hk_karte_leeren (HK_KARTE * k)
{
    k->ms       = 0;
    k->belegt   = 0;
    k->frei     = 0;
    k->fsum     = 0;
    k->gross_b  = HK_NIX;
    k->gross_n  = 0;
    k->zweit_b  = HK_NIX;
    k->zweit_n  = 0;
    k->davor_b  = HK_NIX;
    k->davor_n  = 0;
    k->danach_b = HK_NIX;
    k->danach_n = 0;
    k->abbruch  = 0;
}

/* M1: Blockgang (design.md 4.2 N1). Rein: liest nur das uebergebene Feld, keine Sperre, keine Ausgabe. Die Sperre setzt
 * der Aufrufer (hk_karte_erfassen ()). Alle Laengen in Bloecken; bei Gleichstand gilt der kleinere Index (der Gang laeuft
 * aufsteigend, ersetzt wird nur bei echt groesser). Wachter: bs, nb, ord, it, sum - jeder liefert abbruch statt Werten.
 * it kann bei intakten nb/ord nicht ausloesen (streng steigend und < Blockzahl); er bleibt als Rueckhalt, falls ord
 * je wegfaellt (c80i prueft genau das).
 */
uint8_t
hk_gang (const void * heap, uint16_t numblocks, uint16_t blockgroesse, HK_KARTE * k)
{
    const HK_BLOCK *    h           = (const HK_BLOCK *) heap;
    uint32_t            summe       = 0;
    uint16_t            schritte    = 0;
    uint16_t            b;
    uint16_t            nx;
    uint16_t            nb;
    uint16_t            len;
    uint16_t            vor_b       = HK_NIX;                               // voriger Eintrag (fuer davor)
    uint16_t            vor_n       = 0;
    uint8_t             vor_frei    = 0;
    uint8_t             nach_gross  = 0;                                    // der vorige Eintrag ist das aktuelle gross
    uint8_t             war_nach_gross;

    hk_karte_leeren (k);

    if (blockgroesse != sizeof (HK_BLOCK))
    {
        return k->abbruch = HK_AB_BS;
    }

    if (numblocks < 2)
    {
        return k->abbruch = HK_AB_NB;
    }

    b = h[0].next & HK_INDEX_MASKE;                                         // erster Eintrag

    if (b >= numblocks)
    {
        return k->abbruch = HK_AB_NB;
    }

    for (;;)
    {
        nx = h[b].next;
        nb = nx & HK_INDEX_MASKE;

        if (nb == 0)                                                        // b ist der Endwaechter
        {
            break;
        }

        if (nb >= numblocks)
        {
            return k->abbruch = HK_AB_NB;
        }

        if (nb <= b)
        {
            return k->abbruch = HK_AB_ORD;
        }

        if (++schritte > numblocks)
        {
            return k->abbruch = HK_AB_IT;
        }

        len             = nb - b;
        summe          += len;
        war_nach_gross  = nach_gross;
        nach_gross      = 0;

        if (nx & HK_FREI_BIT)
        {
            k->frei++;
            k->fsum += len;

            if (len > k->gross_n)
            {
                k->zweit_b  = k->gross_b;
                k->zweit_n  = k->gross_n;
                k->gross_b  = b;
                k->gross_n  = len;
                k->davor_b  = vor_frei ? HK_NIX : vor_b;
                k->davor_n  = vor_frei ? 0 : vor_n;
                k->danach_b = HK_NIX;
                k->danach_n = 0;
                nach_gross  = 1;
            }
            else if (len > k->zweit_n)
            {
                k->zweit_b  = b;
                k->zweit_n  = len;
            }
        }
        else
        {
            k->belegt++;

            if (war_nach_gross)
            {
                k->danach_b = b;
                k->danach_n = len;
            }
        }

        vor_b       = b;
        vor_n       = len;
        vor_frei    = (nx & HK_FREI_BIT) ? 1 : 0;
        b           = nb;
    }

    if (summe != (uint32_t) numblocks - 2)                                  // Kopf und Endwaechter zaehlen nicht
    {
        return k->abbruch = HK_AB_SUM;
    }

    return 0;
}

/* M4: Zeiger -> Blockindex wie umm_malloc.cpp:629, (ptr - heap) / 8. HK_NIX ohne Zeiger, HK_FREMD ausserhalb des Haufens. */
uint16_t
hk_index (const void * p, const void * heap, uint16_t numblocks)
{
    const uint8_t * q = (const uint8_t *) p;
    const uint8_t * h = (const uint8_t *) heap;

    if (! q)
    {
        return HK_NIX;
    }

    if (q < h || q >= h + (size_t) numblocks * sizeof (HK_BLOCK))
    {
        return HK_FREMD;
    }

    return (uint16_t) ((size_t) (q - h) / sizeof (HK_BLOCK));
}

/* M2: ein Topf. netto saettigend (Ueberlauf ist bei int32_t undefiniert), rueckgang und sein ms nur bei ECHT groesserem
 * Rueckgang - bei Gleichstand bleibt der erste.
 */
void
hk_topf_buchen (HK_TOPF * t, int32_t delta, uint16_t ms)
{
    if (delta < 0 ? t->netto >= INT32_MIN - delta : t->netto <= INT32_MAX - delta)
    {
        t->netto += delta;
    }
    else
    {
        t->netto = delta < 0 ? INT32_MIN : INT32_MAX;
    }

    uint32_t    rueck = delta < 0 ? (uint32_t) 0 - (uint32_t) delta : 0;   // ohne Vorzeichenueberlauf

    if (rueck > t->rueckgang)
    {
        t->rueckgang    = rueck > 0xFFFFUL ? (uint16_t) 0xFFFF : (uint16_t) rueck;
        t->ms           = ms;
    }
}

/* M2: unmittelbar nach einem Topf. frei ist der Wert NACH dem Aufruf, z->letzter der Wert davor. */
void
hk_nach (HK_ZUSTAND * z, uint8_t topf, uint16_t frei)
{
    hk_topf_buchen (&z->topf[topf], (int32_t) frei - (int32_t) z->letzter, z->ms_lauf);
    z->letzter = frei;
}

/* Am Anfang jedes Durchlaufs (design.md 4.3 N2, Abfolge): sys abschliessen, S, F, X, T pruefen. frei ist zugleich
 * "vorher" fuer wif. aktive = PCBs in tcp_active_pcbs, tw = PCBs in tcp_tw_pcbs, acc = Annahmezaehler aus http.cpp.
 * Liefert die jetzt zu erfassenden Karten bzw. Wegmarken (HK_E_*); der Aufrufer erfasst sie und meldet hk_erfasst ().
 */
uint8_t
hk_anfang (HK_ZUSTAND * z, uint32_t jetzt, uint16_t frei, uint8_t aktive, uint8_t tw, uint16_t acc)
{
    uint8_t     e = 0;
    uint32_t    ms;

    if (! (z->merker & HK_M_GESTARTET))                                     // erster Durchlauf: t0, free0, Karte S
    {
        z->merker   = HK_M_GESTARTET | HK_M_W | HK_M_LAEUFT;
        z->t0       = jetzt;
        z->n        = 1;
        z->free0    = frei;
        z->letzter  = frei;
        z->ms_lauf  = 0;
        z->acc_vor  = acc;
        z->tw_vor   = tw;
        return HK_E_S;
    }

    ms = jetzt - z->t0;

    if (z->merker & HK_M_W)
    {
        hk_topf_buchen (&z->topf[HK_TOPF_SYS], (int32_t) frei - (int32_t) z->letzter, hk_ms16 (ms));
        z->letzter = frei;

        if (ms >= HK_W_MS)                                                  // erster Durchlauf nach W: nichts mehr messen
        {
            z->merker  &= (uint8_t) ~HK_M_W;
            z->free1    = frei;
            z->offen   |= HK_O_KOPF | HK_O_TOPF1 | HK_O_TOPF2 | HK_O_TOPF3 | HK_O_TOPF4 | HK_O_S | HK_O_MARKEN;
            z->offen   |= (z->merker & HK_M_F) ? HK_O_F : 0;
            z->offen   |= (z->merker & HK_M_X) ? HK_O_X : 0;
            z->offen   |= (z->merker & HK_M_T) ? HK_O_T : 0;

            if (! (z->merker & HK_M_MARKEN))                                // X blieb aus: Wegmarken jetzt (Anlass W)
            {
                e |= HK_E_MARKEN;
            }
        }
        else
        {
            z->n++;
            z->ms_lauf = hk_ms16 (ms);
        }
    }

    if (z->merker & HK_M_LAEUFT)
    {
        if (ms >= HK_EREIGNIS_MS)
        {
            z->merker &= (uint8_t) ~HK_M_LAEUFT;
            return e;
        }

        if (! (z->merker & HK_M_F))                                         // F: ein Durchlauf ohne neue Annahme danach
        {
            if (acc != z->acc_vor)
            {
                z->merker |= HK_M_ANGENOMMEN;
            }
            else if (z->merker & HK_M_ANGENOMMEN)
            {
                e |= HK_E_F;
            }

            z->acc_vor = acc;
        }

        if (! (z->merker & HK_M_X) && (z->merker & HK_M_W) && aktive == 0 && (uint32_t) frei + HK_X_ABSTAND < z->free0)
        {
            e |= HK_E_X;

            if (! (z->merker & HK_M_MARKEN))
            {
                e |= HK_E_MARKEN;
            }
        }

        if (! (z->merker & HK_M_T) && z->tw_vor && ! tw)                   // T: TIME_WAIT von >= 1 auf 0
        {
            e |= HK_E_T;
        }

        z->tw_vor = tw;
    }

    return e;
}

/* Nach dem Erfassen: Merker setzen, ms eintragen, Zeilen freigeben. In W wird nichts freigegeben - das geschieht am
 * Ende von W fuer alles, was bis dahin erfasst ist (hk_anfang ()).
 */
void
hk_erfasst (HK_ZUSTAND * z, uint8_t e, uint32_t ms)
{
    uint16_t    frei_geben = (z->merker & HK_M_W) ? 0 : 0xFFFF;

    if (e & HK_E_S)
    {
        z->karte[HK_K_S].ms = ms;
    }

    if (e & HK_E_F)
    {
        z->karte[HK_K_F].ms = ms;
        z->merker          |= HK_M_F;
        z->offen           |= HK_O_F & frei_geben;
    }

    if (e & HK_E_X)
    {
        z->karte[HK_K_X].ms = ms;
        z->merker          |= HK_M_X;
        z->offen           |= HK_O_X & frei_geben;
    }

    if (e & HK_E_T)
    {
        z->karte[HK_K_T].ms = ms;
        z->merker          |= HK_M_T;
        z->offen           |= HK_O_T & frei_geben;
    }

    if (e & HK_E_MARKEN)
    {
        z->merker          |= HK_M_MARKEN;
        z->offen           |= HK_O_MARKEN & frei_geben;
    }

    if (e & HK_E_M)
    {
        z->offen           |= HK_O_M & frei_geben;
    }
}

/* Zu jeder Heap-Zeile: sie zaehlt fuer den Takt (keine Zeile des Instruments im selben Durchlauf und erst 1 s danach).
 * Liefert 1, wenn eine m-Karte zu erfassen ist - nur nach W (AKI.2 N2 (v)).
 */
uint8_t
hk_heapzeile (HK_ZUSTAND * z, uint32_t jetzt)
{
    z->takt = jetzt;
    return (z->merker & (HK_M_GESTARTET | HK_M_W)) == HK_M_GESTARTET;
}

/* Anhaengen mit Kappung: pos bleibt <= size - 1, der Puffer ist immer abgeschlossen. */
static void
hk_anh (char * buf, size_t size, size_t * pos, PGM_P fmt, ...)
{
    va_list     ap;
    int         r;

    if (*pos + 1 >= size)
    {
        return;
    }

    va_start (ap, fmt);
    r = vsnprintf_P (buf + *pos, size - *pos, fmt, ap);
    va_end (ap);

    if (r > 0)
    {
        *pos = (*pos + (size_t) r < size) ? *pos + (size_t) r : size - 1;
    }
}

/* "<blk>+<laenge>" bzw. "-"; Laenge in Bloecken x 8 (AKI.2 N1 (i)) */
static void
hk_feld (char * buf, size_t size, size_t * pos, PGM_P name, uint16_t b, uint16_t n)
{
    hk_anh (buf, size, pos, name);

    if (b == HK_NIX || ! n)
    {
        hk_anh (buf, size, pos, PSTR ("-"));
    }
    else
    {
        hk_anh (buf, size, pos, PSTR ("%u+%lu"), (unsigned) b, (unsigned long) n * 8UL);
    }
}

/* Blockindex einer Wegmarke: Zahl, "-" (gibt es nicht) oder "x" (ausserhalb des Haufens) */
static void
hk_blk (char * buf, size_t size, size_t * pos, uint16_t b)
{
    if (b == HK_NIX)
    {
        hk_anh (buf, size, pos, PSTR ("-"));
    }
    else if (b == HK_FREMD)
    {
        hk_anh (buf, size, pos, PSTR ("x"));
    }
    else
    {
        hk_anh (buf, size, pos, PSTR ("%u"), (unsigned) b);
    }
}

static const char hk_gruende[] PROGMEM = "?\0\0\0bs\0\0nb\0\0ord\0it\0\0sum\0";             // je 4 Byte, Index = HK_AB_*
static const char hk_topfnamen[] PROGMEM = "wifsrvotaupdudpntpvarovrhlgstmsys";              // je 3 Zeichen, Index = HK_TOPF_*

static size_t
hk_zeile_karte (const HK_KARTE * k, char anlass, uint8_t mit_ms, char * buf, size_t size)
{
    size_t  pos = 0;
    char    grund[4];

    buf[0] = '\0';
    hk_anh (buf, size, &pos, PSTR ("- heap karte %c"), anlass);

    if (k->abbruch)
    {
        memcpy_P (grund, hk_gruende + 4 * (k->abbruch <= HK_AB_SUM ? k->abbruch : 0), sizeof (grund));
        grund[3] = '\0';
        hk_anh (buf, size, &pos, PSTR (" abbruch=%s"), grund);
    }
    else
    {
        hk_anh (buf, size, &pos, PSTR (" ent=%u/%u fsum=%lu"), (unsigned) k->belegt, (unsigned) k->frei, (unsigned long) k->fsum * 8UL);
        hk_feld (buf, size, &pos, PSTR (" gross="), k->gross_b, k->gross_n);
        hk_feld (buf, size, &pos, PSTR (" zweit="), k->zweit_b, k->zweit_n);
        hk_feld (buf, size, &pos, PSTR (" davor="), k->davor_b, k->davor_n);
        hk_feld (buf, size, &pos, PSTR (" danach="), k->danach_b, k->danach_n);
    }

    if (mit_ms)
    {
        hk_anh (buf, size, &pos, PSTR (" ms=%lu"), (unsigned long) k->ms);
    }

    return pos;
}

static size_t
hk_zeile_topf (const HK_ZUSTAND * z, uint8_t zeile, char * buf, size_t size)
{
    size_t      pos = 0;
    uint8_t     i;
    char        name[4];

    buf[0] = '\0';
    hk_anh (buf, size, &pos, PSTR ("- heap topf %u/4"), (unsigned) zeile + 1);

    for (i = 3 * zeile; i < 3 * zeile + 3 && i < HK_TOEPFE; i++)
    {
        memcpy_P (name, hk_topfnamen + 3 * i, 3);
        name[3] = '\0';
        hk_anh (buf, size, &pos, PSTR (" %s=%ld/%u@"), name, (long) z->topf[i].netto, (unsigned) z->topf[i].rueckgang);

        if (z->topf[i].rueckgang)
        {
            hk_anh (buf, size, &pos, PSTR ("%u"), (unsigned) z->topf[i].ms);
        }
        else
        {
            hk_anh (buf, size, &pos, PSTR ("-"));
        }
    }

    return pos;
}

/* Wegmarken (design.md 4.5 N2): tcp zuerst, dann die festen Felder, dann udp. Ein Listeneintrag kommt nur hinein, wenn
 * danach noch die festen Felder und " +<n>" (hoechstens 5 Zeichen) Platz haben; was fehlt, zaehlt +<n> am Ende.
 */
#define HK_REST_PLATZ               5

static size_t
hk_zeile_marken (const HK_MARKEN * m, char * buf, size_t size)
{
    char        fest[48];
    char        eintrag[24];
    size_t      fpos = 0;
    size_t      epos;
    size_t      pos = 0;
    size_t      grenze;
    uint8_t     i;
    uint8_t     zeige_tcp = m->tcp_n < HK_MARKEN_TCP ? m->tcp_n : HK_MARKEN_TCP;
    uint8_t     zeige_udp = m->udp_n < HK_MARKEN_UDP ? m->udp_n : HK_MARKEN_UDP;
    uint8_t     gezeigt;
    uint16_t    weg = 0;

    fest[0] = '\0';
    hk_anh (fest, sizeof (fest), &fpos, PSTR (" lst="));
    hk_blk (fest, sizeof (fest), &fpos, m->lst);
    hk_anh (fest, sizeof (fest), &fpos, PSTR (" wet="));
    hk_blk (fest, sizeof (fest), &fpos, m->wet);
    hk_anh (fest, sizeof (fest), &fpos, PSTR (" dhcp="));
    hk_blk (fest, sizeof (fest), &fpos, m->dhcp);
    hk_anh (fest, sizeof (fest), &fpos, PSTR (" eep="));
    hk_blk (fest, sizeof (fest), &fpos, m->eep);

    grenze = size - 1;                                                      // hoechstens so viele Zeichen

    buf[0] = '\0';
    hk_anh (buf, size, &pos, PSTR ("- heap marken %c tcp="), m->anlass);

    for (i = 0, gezeigt = 0; i < zeige_tcp; i++)
    {
        epos = 0;
        eintrag[0] = '\0';

        if (gezeigt)
        {
            hk_anh (eintrag, sizeof (eintrag), &epos, PSTR (","));
        }

        hk_blk (eintrag, sizeof (eintrag), &epos, m->tcp_pcb[i]);
        hk_anh (eintrag, sizeof (eintrag), &epos, PSTR (":%u/"), (unsigned) m->tcp_zst[i]);
        hk_blk (eintrag, sizeof (eintrag), &epos, m->tcp_ctx[i]);

        if (pos + epos + fpos + 5 + 1 + HK_REST_PLATZ > grenze)             // + " udp=" und mindestens "-"
        {
            break;
        }

        hk_anh (buf, size, &pos, PSTR ("%s"), eintrag);
        gezeigt++;
    }

    if (! gezeigt)
    {
        hk_anh (buf, size, &pos, PSTR ("-"));
    }

    weg += m->tcp_n - gezeigt;

    hk_anh (buf, size, &pos, PSTR ("%s udp="), fest);

    for (i = 0, gezeigt = 0; i < zeige_udp; i++)
    {
        epos = 0;
        eintrag[0] = '\0';

        if (gezeigt)
        {
            hk_anh (eintrag, sizeof (eintrag), &epos, PSTR (","));
        }

        hk_blk (eintrag, sizeof (eintrag), &epos, m->udp_pcb[i]);
        hk_anh (eintrag, sizeof (eintrag), &epos, PSTR ("/"));
        hk_blk (eintrag, sizeof (eintrag), &epos, m->udp_ctx[i]);

        if (pos + epos + HK_REST_PLATZ > grenze)
        {
            break;
        }

        hk_anh (buf, size, &pos, PSTR ("%s"), eintrag);
        gezeigt++;
    }

    if (! gezeigt)
    {
        hk_anh (buf, size, &pos, PSTR ("-"));
    }

    weg += m->udp_n - gezeigt;

    if (weg)
    {
        hk_anh (buf, size, &pos, PSTR (" +%u"), (unsigned) weg);
    }

    return pos;
}

/* Eine Zeile nach ihrem Bit in offen (HK_O_*). Ohne switch: eine Sprungtabelle laege im DRAM (.rodata). */
size_t
hk_zeile (const HK_ZUSTAND * z, uint8_t bit, char * buf, size_t size)
{
    size_t  pos = 0;

    if (bit == 0)
    {
        buf[0] = '\0';
        hk_anh (buf, size, &pos, PSTR ("- heap topf kopf t0=%lu w=%lu n=%lu free0=%u free1=%u us=%u"),
                (unsigned long) z->t0, (unsigned long) HK_W_MS, (unsigned long) z->n,
                (unsigned) z->free0, (unsigned) z->free1, (unsigned) z->us);
        return pos;
    }

    if (bit <= 4)
    {
        return hk_zeile_topf (z, bit - 1, buf, size);
    }

    if (bit == 5)
    {
        return hk_zeile_karte (&z->karte[HK_K_S], 'S', 1, buf, size);
    }

    if (bit == 6)
    {
        return hk_zeile_karte (&z->karte[HK_K_F], 'F', 1, buf, size);
    }

    if (bit == 7)
    {
        return hk_zeile_karte (&z->karte[HK_K_X], 'X', 1, buf, size);
    }

    if (bit == 8)
    {
        return hk_zeile_marken (&z->marken, buf, size);
    }

    if (bit == 9)
    {
        return hk_zeile_karte (&z->karte[HK_K_T], 'T', 1, buf, size);
    }

    return hk_zeile_karte (&z->karte[HK_K_M], 'm', 0, buf, size);
}

/* Takt der Ausgabe (AKI.1 (e) N2): nichts vor dem Start und nichts in W; danach hoechstens eine Zeile je HK_TAKT_MS,
 * gezaehlt ab der letzten Zeile des Instruments ODER der letzten Heap-Zeile. Liefert die Laenge, 0 = keine Zeile.
 */
size_t
hk_ausgabe (HK_ZUSTAND * z, uint32_t jetzt, char * buf, size_t size)
{
    uint8_t     bit;

    if ((z->merker & (HK_M_GESTARTET | HK_M_W)) != HK_M_GESTARTET || ! (z->offen & HK_O_ALLE))
    {
        return 0;
    }

    if ((uint32_t) (jetzt - z->takt) < HK_TAKT_MS)
    {
        return 0;
    }

    for (bit = 0; ! (z->offen & (1U << bit)); bit++)
    {
        ;
    }

    z->offen   &= (uint16_t) ~(1U << bit);
    z->takt     = jetzt;
    return hk_zeile (z, bit, buf, size);
}

/*== C80I REIN ENDE =====================================================================================================================*/

/*----------------------------------------------------------------------------------------------------------------------------------------
 * Geraeteteil
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
HK_ZUSTAND                  heapkarte;                                      // das gesamte BSS des Instruments (304 Byte)

extern WiFiClient           openweather_client;                             // weather.cpp:16, nur gelesen

/* Zugriffsglied auf WiFiClient::_client (geschuetzt): Der Zeiger auf das Glied entsteht in der Unterklasse und wird auf
 * das unveraenderte Objekt aus weather.cpp angewandt. Kein Objekt dieser Klasse wird angelegt; weather.cpp bleibt.
 */
class HkWetterZugriff : public WiFiClient
{
    public:
        static const void * kontext (const WiFiClient & c)
        {
            return (const void *) (c.*(&HkWetterZugriff::_client));
        }
};

/* Blockgang unter derselben Sperre wie umm_info () (UMM_CRITICAL_ENTRY = xt_rsil (15), umm_malloc_cfg.h:459-460).
 * Zwischen Sperren und Freigeben steht genau ein Aufruf und KEINE Verzweigung: Die Sperre wird auf jedem Weg geloest,
 * auch bei abbruch=. Bewusst kein RAII (design.md 4.2 N1). Dauer in us, laengste seit dem Start.
 */
static void
hk_karte_erfassen (HK_KARTE * k)
{
    umm_heap_context_t *    c   = umm_get_current_heap ();
    uint16_t                bs  = (uint16_t) umm_block_size ();
    uint32_t                t   = micros ();
    uint32_t                ps;

    ps = xt_rsil (15);
    hk_gang ((const void *) c->heap, (uint16_t) c->numblocks, bs, k);
    xt_wsr_ps (ps);

    t = micros () - t;

    if (t > heapkarte.us)
    {
        heapkarte.us = t > 0xFFFFUL ? (uint16_t) 0xFFFF : (uint16_t) t;
    }
}

/* M4: Wegmarken, nur lesend, ohne CPU-Abgabe (die lwIP-Listen aendern sich nur, wenn loop () abgibt). tcp: aktive, dann
 * TIME_WAIT - bei X ist tcp_active_pcbs leer, es sind also genau die PCBs in tcp_tw_pcbs (wie http.cpp:745). Ein PCB in
 * TIME_WAIT hat keinen ClientContext mehr (callback_arg NULL), ctx steht dann "-". Keine Gegenstelle (AKI.1 (g)).
 */
static void
hk_marken_tcp (HK_MARKEN * m, const struct tcp_pcb * pcb, const void * heap, uint16_t nb)
{
    for (; pcb; pcb = pcb->next)
    {
        if (m->tcp_n < HK_MARKEN_TCP)
        {
            m->tcp_pcb[m->tcp_n] = hk_index (pcb, heap, nb);
            m->tcp_ctx[m->tcp_n] = hk_index (pcb->callback_arg, heap, nb);
            m->tcp_zst[m->tcp_n] = (uint8_t) pcb->state;
        }

        if (m->tcp_n < 0xFF)
        {
            m->tcp_n++;
        }
    }
}

static void
hk_marken_erfassen (HK_MARKEN * m, char anlass)
{
    umm_heap_context_t *        c   = umm_get_current_heap ();
    const void *                h   = (const void *) c->heap;
    uint16_t                    nb  = (uint16_t) c->numblocks;
    const struct udp_pcb *      u;
    const struct tcp_pcb_listen * l;
    struct netif *              n;
    uint8_t                     i;

    m->anlass   = anlass;
    m->tcp_n    = 0;
    m->udp_n    = 0;
    m->lst      = HK_NIX;
    m->dhcp     = HK_NIX;

    for (i = 0; i < HK_MARKEN_TCP; i++)
    {
        m->tcp_pcb[i] = m->tcp_ctx[i] = HK_NIX;
        m->tcp_zst[i] = 0;
    }

    for (i = 0; i < HK_MARKEN_UDP; i++)
    {
        m->udp_pcb[i] = m->udp_ctx[i] = HK_NIX;
    }

    hk_marken_tcp (m, tcp_active_pcbs, h, nb);
    hk_marken_tcp (m, tcp_tw_pcbs, h, nb);

    for (u = udp_pcbs; u; u = u->next)
    {
        if (m->udp_n < HK_MARKEN_UDP)
        {
            m->udp_pcb[m->udp_n] = hk_index (u, h, nb);
            m->udp_ctx[m->udp_n] = hk_index (u->recv_arg, h, nb);           // UdpContext (WiFiUdp), genau
        }

        if (m->udp_n < 0xFF)
        {
            m->udp_n++;
        }
    }

    for (l = tcp_listen_pcbs.listen_pcbs; l; l = l->next)
    {
        if (l->local_port == 80)
        {
            m->lst = hk_index (l, h, nb);
            break;
        }
    }

    for (n = netif_list; n; n = n->next)
    {
        if (netif_dhcp_data (n))
        {
            m->dhcp = hk_index (netif_dhcp_data (n), h, nb);
            break;
        }
    }

    m->wet = hk_index (HkWetterZugriff::kontext (openweather_client), h, nb);
    m->eep = hk_index (EEPROM.getConstDataPtr (), h, nb);                   // NICHT getDataPtr (): das setzt _dirty
}

/* Am Anfang jedes Durchlaufs bis 180 s (HEAPKARTE_ANFANG ()). free zuerst: Alles danach bis wif belegt nichts. */
void
heapkarte_anfang (void)
{
    uint16_t                frei    = (uint16_t) ESP.getFreeHeap ();
    uint32_t                jetzt   = millis ();
    const struct tcp_pcb *  pcb;
    uint8_t                 aktive  = 0;
    uint8_t                 tw      = 0;
    uint8_t                 e;
    uint8_t                 k;
    const HK_KARTE *        quelle  = (const HK_KARTE *) 0;

    for (pcb = tcp_active_pcbs; pcb && aktive < 0xFF; pcb = pcb->next)
    {
        aktive++;
    }

    for (pcb = tcp_tw_pcbs; pcb && tw < 0xFF; pcb = pcb->next)
    {
        tw++;
    }

    e = hk_anfang (&heapkarte, jetzt, frei, aktive, tw, http_accept_count_get ());

    if (! e)
    {
        return;
    }

    for (k = HK_K_S; k <= HK_K_T; k++)                                      // HK_E_S..HK_E_T in derselben Reihenfolge
    {
        if (e & (1U << (k - HK_K_S)))
        {
            if (quelle)                                                     // zwei Ereignisse im selben Augenblick:
            {                                                               // ein Gang genuegt
                heapkarte.karte[k] = *quelle;
            }
            else
            {
                hk_karte_erfassen (&heapkarte.karte[k]);
                quelle = &heapkarte.karte[k];
            }
        }
    }

    if (e & HK_E_MARKEN)
    {
        hk_marken_erfassen (&heapkarte.marken, (e & HK_E_X) ? 'X' : 'W');
    }

    hk_erfasst (&heapkarte, e, jetzt - heapkarte.t0);
}

/* Aus esp_heap_log (), im selben Fenster ohne CPU-Abgabe wie free=/max= der Heap-Zeile: Dann gilt gross = max und
 * fsum = free exakt (AKI.3 N1).
 */
void
heapkarte_heapzeile (void)
{
    if (hk_heapzeile (&heapkarte, millis ()))
    {
        hk_karte_erfassen (&heapkarte.karte[HK_K_M]);
        hk_erfasst (&heapkarte, HK_E_M, 0);
    }
}

/* Einmal formatieren, zweimal ausgeben - Mitschnitt und /api/stm32_log (L185), wie esp_heap_log (). */
void
heapkarte_ausgabe (void)
{
    char    line[120];                                                      // STM32_LOG_LINE_LEN: hoechstens 119 Zeichen

    if (hk_ausgabe (&heapkarte, millis (), line, sizeof (line)))
    {
        Serial.println (line);
        Serial.flush ();
        stm32_log_append (line);
    }
}

#endif // ESP_HEAP_MAP
