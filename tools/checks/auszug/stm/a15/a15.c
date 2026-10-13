/* Pruefstand a15 (A15, Paket 2026-10-13, AKI.1). Alte ISR (Stand ec19f33, SPL-Aufrufe) und neue ISR
 * (direkte Registerzugriffe) laufen gegen dieselbe Registerattrappe, je mit eigenem Kontext:
 *   - DR lesen loescht RXNE; ORE nur, wenn SR davor gelesen wurde (Folge "SR, dann DR")
 *   - DR schreiben loescht TXE; TXE ist nur lesbar
 *   - SR schreiben: rc_w0-Bits (RXNE, TC, LBD, CTS) werden durch 0 geloescht, der Rest ist nur lesbar
 *   - CR1 normal les- und schreibbar; uart_putc setzt TXEIE per Lese-Aendere-Schreibe wie USART_ITConfig
 * Die SPL-Funktionen der alten ISR sind 1:1 nach stm32f10x_usart.c / stm32f4xx_usart.c nachgebaut.
 * Verglichen wird nach JEDEM Schritt: SR, CR1, Empfangsring (logischer Inhalt), Indizes, rxsize, rxmax,
 * rxdrops, rxore, txsize, gesendete Zeichen (Zahl und Pruefsumme). Dazu absolute Sollwerte je Randfall. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pruefstand.h"

#ifndef RXN
#define RXN 8
#endif
#ifndef TXN
#define TXN 4
#endif
#define UART_RXBUFLEN RXN
#define UART_TXBUFLEN TXN

#define SR_TC   0x0040
#define SR_LBD  0x0100
#define SR_CTS  0x0200
#define SR_RC_W0 (USART_SR_RXNE | SR_TC | SR_LBD | SR_CTS)
#define CR1_REST 0x200C                                /* UE, TE, RE: muessen jede CR1-Aenderung ueberleben */

typedef enum { RESET = 0, SET = !RESET } ITStatus;
typedef enum { DISABLE = 0, ENABLE = !DISABLE } FunctionalState;

typedef struct
{
    uint16_t            SR, CR1, CR2, CR3, rdr;
    int                 sr_gelesen;
    volatile uint8_t    rxbuf[RXN];
    volatile uint_fast16_t rxsize, rxmax;
    volatile uint16_t   rxdrops, rxore;
    volatile uint8_t    txbuf[TXN];
    volatile uint_fast16_t txsize;
    uint_fast16_t       rxstop, txstart;               /* static in der ISR */
    uint_fast16_t       rxstart, txstop;               /* Hauptloop-Seite */
    uint32_t            gesendet, summe;
    long                sr_lesungen, dr_lesungen;
} KTX;

static uint16_t rd_SR  (KTX * k) { k->sr_lesungen++; k->sr_gelesen = 1; return k->SR; }
static uint16_t rd_CR1 (KTX * k) { return k->CR1; }
static uint16_t rd_CR2 (KTX * k) { return k->CR2; }
static uint16_t rd_CR3 (KTX * k) { return k->CR3; }
static uint16_t rd_DR  (KTX * k)
{
    k->dr_lesungen++;
    k->SR &= ~USART_SR_RXNE;
    if (k->sr_gelesen) k->SR &= ~USART_SR_ORE;
    k->sr_gelesen = 0;
    return k->rdr;
}
static void wr_DR  (KTX * k, uint32_t v) { k->gesendet++; k->summe = k->summe * 31 + (v & 0x1FF) + 1; k->SR &= ~USART_SR_TXE; }
static void wr_SR  (KTX * k, uint32_t v) { k->SR &= (uint16_t) (~SR_RC_W0 | v); }
static void wr_CR1 (KTX * k, uint32_t v) { k->CR1 = (uint16_t) v; }

/* SPL, nachgebaut nach stm32f10x_usart.c / stm32f4xx_usart.c (beide identisch fuer diese Funktionen) */
static ITStatus USART_GetITStatus (KTX * x, uint16_t it)
{
    uint32_t usartreg = ((uint8_t) it) >> 5, itmask = (uint32_t) 1 << (it & 0x1F), bitpos;
    if (usartreg == 1) itmask &= rd_CR1 (x); else if (usartreg == 2) itmask &= rd_CR2 (x); else itmask &= rd_CR3 (x);
    bitpos = (uint32_t) 1 << (it >> 8);
    bitpos &= rd_SR (x);
    return (itmask != 0 && bitpos != 0) ? SET : RESET;
}
static void USART_ClearITPendingBit (KTX * x, uint16_t it) { uint16_t m = (uint16_t) 1 << (it >> 8); wr_SR (x, (uint16_t) ~m); }
static uint16_t USART_ReceiveData (KTX * x) { return (uint16_t) (rd_DR (x) & 0x01FF); }
static void USART_SendData (KTX * x, uint16_t d) { wr_DR (x, d & 0x01FF); }
static void USART_ITConfig (KTX * x, uint16_t it, FunctionalState s)
{
    uint32_t m = (uint32_t) 1 << (it & 0x1F);
    if (((uint8_t) it >> 5) != 1) { printf ("  FEHL  ITConfig ausserhalb CR1\n"); return; }
    if (s != DISABLE) wr_CR1 (x, rd_CR1 (x) | m); else wr_CR1 (x, rd_CR1 (x) & ~m);
}

#define UART_NAME K
#include "isr.inc"
#undef UART_NAME

/* ---- Hauptloop-Seite und Hardware-Ereignisse, fuer beide Kontexte identisch ---- */
static void ankunft (KTX * k, uint16_t c)                  /* ein Zeichen kommt im Schieberegister an */
{
    if (k->SR & USART_SR_RXNE) k->SR |= USART_SR_ORE;      /* DR noch voll: Zeichen geht verloren, ORE */
    else { k->rdr = c; k->SR |= USART_SR_RXNE; }
}
static void putc_haupt (KTX * k, uint8_t c)                /* wie uart_putc: Ring, dann TXEIE per RMW */
{
    if (k->txsize >= TXN) return;                          /* Firmware wartet hier; der Pruefstand laesst aus */
    k->txbuf[k->txstop++] = c; if (k->txstop >= TXN) k->txstop = 0;
    k->txsize++;
    USART_ITConfig (k, USART_IT_TXE, ENABLE);
}
static int getc_haupt (KTX * k)
{
    if (k->rxsize == 0) return -1;
    int c = k->rxbuf[k->rxstart++]; if (k->rxstart >= RXN) k->rxstart = 0;
    k->rxsize--; return c;
}
static void init (KTX * k, uint16_t cr1)
{
    memset (k, 0, sizeof *k);
    k->SR = USART_SR_TXE | SR_TC; k->CR1 = cr1;
}

static int fails, cases, abweichungen;
#define CHECK(name, cond) do { cases++; if (cond) printf ("  OK    %s\n", name); else { fails++; printf ("  FEHL  %s\n", name); } } while (0)

static int gleich (KTX * a, KTX * b, char * w, size_t n)
{
    #define V(f) if (a->f != b->f) { snprintf (w, n, #f " alt %lu, neu %lu", (unsigned long) a->f, (unsigned long) b->f); return 0; }
    V(SR) V(CR1) V(rxsize) V(rxmax) V(rxdrops) V(rxore) V(txsize) V(rxstop) V(txstart) V(gesendet) V(summe) V(rdr)
    for (uint_fast16_t i = 0; i < a->rxsize; i++)
        if (a->rxbuf[(a->rxstart + i) % RXN] != b->rxbuf[(b->rxstart + i) % RXN]) { snprintf (w, n, "Empfangsring Position %u", (unsigned) i); return 0; }
    return 1;
    #undef V
}

/* ---- Zufallsfolgen ---- */
static uint32_t rng = 0x1234567u;
static uint32_t zz (void) { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }

static void zufall (int folgen, int schritte)
{
    KTX a, b; char w[160]; int erste = 1; long sr_a = 0, sr_b = 0, dr_a = 0, dr_b = 0, isr = 0, ore = 0, drops = 0, aus = 0, gesendet = 0;
    for (int f = 0; f < folgen; f++)
    {
        uint16_t cr1 = CR1_REST | ((zz () % 10) ? USART_CR1_RXNEIE : 0);
        init (&a, cr1); init (&b, cr1);
        uint32_t lastw = zz () % 4;                                  /* Lastprofil je Folge: wenig bis Flut */
        int steht = (f % 50) == 0;                                   /* Hauptloop steht (L204): kein getc, lange Folge */
        int lang = steht ? 3 * RXN + 200 : schritte;
        for (int s = 0; s < lang; s++)
        {
            uint32_t r = zz ();
            int an = (r % 8) < lastw + 1 ? 1 + (int) ((r >> 3) % (1 + lastw)) : 0;
            for (int i = 0; i < an; i++) { uint16_t c = zz () & 0x1FF; ankunft (&a, c); ankunft (&b, c); }
            if (((r >> 6) % 3) == 0) { a.SR |= USART_SR_TXE; b.SR |= USART_SR_TXE; }          /* Schieber fertig */
            if (((r >> 8) % 61) == 0) { a.SR |= USART_SR_ORE; b.SR |= USART_SR_ORE; }         /* ORE allein */
            if (((r >> 14) % 97) == 0) { a.CR1 ^= USART_CR1_RXNEIE; b.CR1 ^= USART_CR1_RXNEIE; }
            int p = (r >> 21) % 4; for (int i = 0; i < p; i++) { uint8_t c = zz (); putc_haupt (&a, c); putc_haupt (&b, c); }
            int g = steht ? 0 : (int) ((r >> 23) % (5 - lastw)); for (int i = 0; i < g; i++) { if (getc_haupt (&a) != getc_haupt (&b) && erste) { abweichungen++; erste = 0; printf ("  FEHL  Folge %d Schritt %d: getc liefert verschieden\n", f, s); } }
            int n = (r >> 26) % 4;                                   /* 0..3 Eintritte, auch ohne Anlass */
            if (steht) n = 2;
            for (int i = 0; i < n; i++) { int vorher = a.CR1 & USART_CR1_TXEIE; isr_alt (&a); isr_neu (&b); isr++; aus += vorher && !(a.CR1 & USART_CR1_TXEIE); }
            if (!gleich (&a, &b, w, sizeof w))
            {
                abweichungen++;
                if (erste) { printf ("  FEHL  Folge %d Schritt %d: %s\n", f, s, w); erste = 0; }
                break;
            }
        }
        sr_a += a.sr_lesungen; sr_b += b.sr_lesungen; dr_a += a.dr_lesungen; dr_b += b.dr_lesungen;
        ore += a.rxore; drops += a.rxdrops; gesendet += a.gesendet;
    }
    char name[200];
    snprintf (name, sizeof name, "AKI.1 %d Zufallsfolgen x %d Schritte (Ring %d/%d): %d Abweichungen", folgen, schritte, RXN, TXN, abweichungen);
    CHECK (name, abweichungen == 0);
    printf ("        abgedeckt: %ld ISR-Eintritte, ORE gezaehlt %ld, Ring-voll-Verwuerfe %ld, gesendet %ld, TXEIE-Abschaltungen %ld\n", isr, ore, drops, gesendet, aus);
    printf ("        SR-Lesungen alt %ld / neu %ld, DR-Lesungen alt %ld / neu %ld\n", sr_a, sr_b, dr_a, dr_b);
    CHECK ("AKI.1 Abdeckung: ORE, Ring voll, Senden und TXE-Abschaltung kamen in den Folgen vor", ore > 0 && drops > 0 && gesendet > 0 && aus > 0);
}

/* ---- Randfaelle: beide ISR, Vergleich UND absolute Sollwerte ---- */
typedef void (*vorbereitung) (KTX *);
static void rand_fall (const char * name, vorbereitung v, int (*soll) (KTX *))
{
    KTX a, b; char w[160], n[220];
    init (&a, CR1_REST | USART_CR1_RXNEIE); init (&b, CR1_REST | USART_CR1_RXNEIE);
    v (&a); v (&b);
    isr_alt (&a); isr_neu (&b);
    int g = gleich (&a, &b, w, sizeof w);
    snprintf (n, sizeof n, "%s: alt == neu%s%s", name, g ? "" : " -- ", g ? "" : w); CHECK (n, g);
    snprintf (n, sizeof n, "%s: Sollwert (neu)", name); CHECK (n, soll (&b));
    snprintf (n, sizeof n, "%s: Sollwert (alt, Referenz)", name); CHECK (n, soll (&a));
}
static void v_rxne (KTX * k) { ankunft (k, 0x141); k->SR &= ~USART_SR_TXE; }
static int  s_rxne (KTX * k) { return k->rxsize == 1 && k->rxbuf[0] == 0x41 && k->rxore == 0 && !(k->SR & USART_SR_RXNE) && k->rxmax == 1 && k->gesendet == 0; }
static void v_ore (KTX * k) { ankunft (k, 'A'); ankunft (k, 'B'); k->SR &= ~USART_SR_TXE; }
static int  s_ore (KTX * k) { return k->rxsize == 1 && k->rxbuf[0] == 'A' && k->rxore == 1 && !(k->SR & (USART_SR_RXNE | USART_SR_ORE)); }
static void v_ore_allein (KTX * k) { k->SR |= USART_SR_ORE; }
static int  s_ore_allein (KTX * k) { return k->rxsize == 0 && k->rxore == 0 && (k->SR & USART_SR_ORE) && k->dr_lesungen == 0; }
static void v_txe_aus (KTX * k) { k->txbuf[0] = 'x'; k->txsize = 1; k->txstop = 1; }   /* Daten da, TXEIE aus */
static int  s_txe_aus (KTX * k) { return k->gesendet == 0 && k->txsize == 1 && k->CR1 == (CR1_REST | USART_CR1_RXNEIE); }
static void v_txe_daten (KTX * k) { putc_haupt (k, 'x'); putc_haupt (k, 'y'); }
static int  s_txe_daten (KTX * k) { return k->gesendet == 1 && k->txsize == 1 && (k->CR1 & USART_CR1_TXEIE) && !(k->SR & USART_SR_TXE); }
static void v_tx_leer (KTX * k) { k->CR1 |= USART_CR1_TXEIE; }
static int  s_tx_leer (KTX * k) { return k->gesendet == 0 && k->CR1 == (CR1_REST | USART_CR1_RXNEIE) && (k->SR & USART_SR_TXE); }
static void v_beide (KTX * k) { putc_haupt (k, 'z'); ankunft (k, 'R'); }
static int  s_beide (KTX * k) { return k->rxsize == 1 && k->rxbuf[0] == 'R' && k->gesendet == 1 && k->txsize == 0 && !(k->SR & (USART_SR_RXNE | USART_SR_TXE)); }
static void v_voll (KTX * k) { k->rxsize = RXN; k->rxmax = RXN; ankunft (k, 'V'); }
static int  s_voll (KTX * k) { return k->rxsize == RXN && k->rxdrops == 1 && k->rxstop == 0 && !(k->SR & USART_SR_RXNE); }
static void v_rxneie_aus (KTX * k) { k->CR1 &= ~USART_CR1_RXNEIE; ankunft (k, 'N'); }
static int  s_rxneie_aus (KTX * k) { return k->rxsize == 0 && (k->SR & USART_SR_RXNE) && k->dr_lesungen == 0; }
static void v_saett (KTX * k) { k->rxore = 0xFFFF; k->rxdrops = 0xFFFF; k->rxsize = RXN; ankunft (k, 'a'); ankunft (k, 'b'); }
static int  s_saett (KTX * k) { return k->rxore == 0xFFFF && k->rxdrops == 0xFFFF; }
static void v_umlauf (KTX * k) { k->rxstop = RXN - 1; k->rxstart = RXN - 1; ankunft (k, 'U'); }
static int  s_umlauf (KTX * k) { return k->rxsize == 1 && k->rxbuf[RXN - 1] == 'U' && k->rxstop == 0; }

int
main (void)
{
    printf ("Typbreiten: uint_fast8_t %u Byte, uint_fast16_t %u Byte; Ring %d/%d\n", (unsigned) sizeof (uint_fast8_t), (unsigned) sizeof (uint_fast16_t), RXN, TXN);
    rand_fall ("RXNE allein (9-Bit-Wert 0x141)", v_rxne, s_rxne);
    rand_fall ("RXNE + ORE", v_ore, s_ore);
    rand_fall ("ORE allein, RXNE aus", v_ore_allein, s_ore_allein);
    rand_fall ("TXE gesetzt, TXEIE aus, Daten im Ring", v_txe_aus, s_txe_aus);
    rand_fall ("TXE + TXEIE, Daten im Ring", v_txe_daten, s_txe_daten);
    rand_fall ("TXE + TXEIE, Sendering leer", v_tx_leer, s_tx_leer);
    rand_fall ("RXNE und TXE zugleich", v_beide, s_beide);
    rand_fall ("Ring voll", v_voll, s_voll);
    rand_fall ("RXNE, RXNEIE aus", v_rxneie_aus, s_rxneie_aus);
    rand_fall ("Zaehler saettigen bei 0xFFFF", v_saett, s_saett);
    rand_fall ("Ringindex laeuft um", v_umlauf, s_umlauf);
    zufall (12000, 60);
    printf ("%d Faelle, %d FEHL\n", cases, fails);
    return fails ? 1 : 0;
}
