/* Pruefstand FR (Paket 2026-10-10, A50, AKF.1 und AKF.2): Weg vom Fault zum Reset.
 *
 * Laesst den ECHTEN fault_reset() samt der vier Handler aus src/main.c laufen (extract.py).
 * Nachgebildet ist nur die Umgebung des HardFault:
 *   - USART-ISR laeuft NICHT (Prioritaet 0 unter -1): uart_putc() wartet bei vollem Ring,
 *     uart_flush() wartet, bis der Ring leer ist -- beides ewig. Ein begrenzter Zaehler meldet "haengt".
 *   - SysTick als Zaehler mit simulierten Kerntakten: Jeder Registerzugriff kostet st_cost Takte,
 *     COUNTFLAG wird gesetzt, wenn seit dem letzten Zugriff ein Ueberlauf lag, und beim Lesen geloescht.
 *     Verpasste Ueberlaeufe (mehr als einer zwischen zwei Zugriffen) werden gezaehlt.
 *   - Der echte Kerntakt f_real steht im RCC; SystemCoreClockUpdate() liest ihn. Bis dahin traegt
 *     SystemCoreClock den Wert des Falls (z. B. den Vorgabewert 168 MHz des F411 vor main()).
 *   - NVIC_SystemReset() springt per longjmp zurueck und haelt den Taktstand fest.
 * Gemessen wird die Wartezeit ab Fault bis Reset in Sekunden: f_real-Takte.
 *
 * Zieltypen (L256): pruefstand.h; der Code rechnet mit uint32_t, -DPRUEFSTAND_SCHNELLE_TYPEN zieht
 * uint_fast*_t zusaetzlich auf 32 Bit.
 */
#include "pruefstand.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <setjmp.h>

static uint64_t     cyc;                    /* simulierte Kerntakte seit dem Fault */
static uint32_t     f_real;                 /* tatsaechlicher Kerntakt laut RCC */
static jmp_buf      jb;
static int          irq_off;
static long         spin;
#define SPIN_LIMIT  50000000L
#define FR_RESET    1
#define FR_HANG     2

/* ---------- CMSIS-Nachbildung ---------- */
uint32_t SystemCoreClock;
static int scu_calls;
void SystemCoreClockUpdate (void) { SystemCoreClock = f_real; scu_calls++; }
#define __disable_irq()     (irq_off = 1)
__attribute__((noreturn)) void NVIC_SystemReset (void) { longjmp (jb, FR_RESET); }

typedef struct { uint32_t CTRL, LOAD, VAL, CALIB; } SysTick_Type;
#define SysTick_CTRL_COUNTFLAG_Msk  (1UL << 16)
#define SysTick_CTRL_CLKSOURCE_Msk  (1UL << 2)
#define SysTick_CTRL_TICKINT_Msk    (1UL << 1)
#define SysTick_CTRL_ENABLE_Msk     (1UL << 0)
static SysTick_Type st;
static uint32_t     st_cost;                /* Takte je Registerzugriff */
static int          st_run, st_own;     /* st_own: Phase vom geprueften Code gestartet */
static uint64_t     st_start, st_seen, st_lost;
static int          st_badload, st_tickint;

static SysTick_Type *
systick_access (void)
{
    uint64_t period, wraps, neu;

    cyc += st_cost;
    if (! (st.CTRL & SysTick_CTRL_ENABLE_Msk)) { st_run = 0; st.CTRL &= ~SysTick_CTRL_COUNTFLAG_Msk; return &st; }
    if (! st_run)                           /* Phase, die der gepruefte Code gestartet hat */
    {
        st_run = 1; st_own = 1; st_start = cyc; st_seen = 0;
        if (st.LOAD > 0xFFFFFFUL || st.LOAD == 0) st_badload = 1;
        if (st.CTRL & SysTick_CTRL_TICKINT_Msk) st_tickint = 1;
    }
    period = (uint64_t) (st.LOAD & 0xFFFFFFUL) + 1;
    if (! (st.CTRL & SysTick_CTRL_CLKSOURCE_Msk)) period *= 8;     /* AHB/8 */
    wraps = (cyc - st_start) / period;
    neu = wraps - st_seen;
    if (neu > 1 && st_own) st_lost += neu - 1;
    st_seen = wraps;
    st.CTRL = (st.CTRL & ~SysTick_CTRL_COUNTFLAG_Msk) | (neu ? SysTick_CTRL_COUNTFLAG_Msk : 0);
    return &st;
}
#define SysTick (systick_access ())

/* Fuer den Ausgangsstand: SCB-Register, die die alte Ausgabe las. */
typedef struct { uint32_t CFSR, HFSR, ICSR, MMFAR, BFAR; } SCB_Type;
static SCB_Type scb __attribute__((unused));
#define SCB (&scb)

/* ---------- Log ohne ISR ---------- */
#define TXLEN 64
static int tx_size;
static void hang_check (void) { cyc += 4; if (++spin > SPIN_LIMIT) longjmp (jb, FR_HANG); }
void log_uart_putc (char c) { (void) c; while (tx_size >= TXLEN) hang_check (); tx_size++; }
void log_uart_puts (const char * s) { while (*s) log_uart_putc (*s++); }
void log_uart_flush (void) { while (tx_size > 0) hang_check (); }
void log_printf (const char * fmt, ...) { char b[256]; va_list ap; va_start (ap, fmt); vsnprintf (b, sizeof (b), fmt, ap); va_end (ap); log_uart_puts (b); }
#define log_message(s)  do { log_uart_puts (s); log_uart_puts ("\r\n"); } while (0)
#define log_puts(s)     do { log_uart_puts (s); } while (0)
#define log_flush()     do { log_uart_flush (); } while (0)

/* ---------- der echte Code ---------- */
#include SNIPPET

/* ---------- Faelle ---------- */
static int fails, cases;
#define CHECK(name, cond) do { cases++; if (cond) printf ("  OK    %s\n", name); else { fails++; printf ("  FEHL  %s\n", name); } } while (0)

typedef struct
{
    const char *    name;
    uint32_t        f_real;         /* Kerntakt laut RCC */
    uint32_t        sysclk_var;     /* SystemCoreClock beim Fault */
    int             tx_fill;        /* Zeichen im TX-Ring beim Fault */
    int             systick_pre;    /* 1: SysTick laeuft wie nach delay_init (5 us, TICKINT) */
    uint32_t        cost;           /* Takte je Pollschritt */
    int             handler;        /* 0..3 */
} FALL;

static void (* const handler_fn[4]) (void) = { HardFault_Handler, MemManage_Handler, BusFault_Handler, UsageFault_Handler };
static const char * const handler_name[4] = { "HardFault", "MemManage", "BusFault", "UsageFault" };

static void
fall (const FALL * f)
{
    char    n[200];
    int     r;
    double  sec;

    cyc = 0; spin = 0; irq_off = 0; scu_calls = 0;
    f_real = f->f_real; SystemCoreClock = f->sysclk_var; tx_size = f->tx_fill;
    memset (&st, 0, sizeof (st)); st_run = 0; st_own = 0; st_start = 0; st_seen = 0; st_lost = 0; st_badload = 0; st_tickint = 0; st_cost = f->cost;
    if (f->systick_pre) { st.LOAD = f->f_real / 200000 - 1; st.CTRL = 7; st_run = 1; st_start = 0; }   /* wie delay_init (5 us, TICKINT) */

    printf ("[%s] %s, f=%u Hz, SystemCoreClock beim Fault %u, Ring %d/%d, Pollschritt %u Takte\n",
            handler_name[f->handler], f->name, (unsigned) f->f_real, (unsigned) f->sysclk_var, f->tx_fill, TXLEN, (unsigned) f->cost);

    r = setjmp (jb);
    if (r == 0) { handler_fn[f->handler] (); r = 0; }
    if (st_run && (st.CTRL & SysTick_CTRL_TICKINT_Msk)) st_tickint = 1;   /* Stand beim Reset */

    sec = (double) cyc / (double) f->f_real;
    snprintf (n, sizeof (n), "erreicht NVIC_SystemReset() (%s)", r == FR_RESET ? "ja" : r == FR_HANG ? "haengt in der Ausgabe ohne ISR" : "?");
    CHECK (n, r == FR_RESET);
    if (r != FR_RESET) return;
    snprintf (n, sizeof (n), "Wartezeit %.4f s: ueber 12 s und unter 20 s", sec);
    CHECK (n, sec > 12.0 && sec < 20.0);
    snprintf (n, sizeof (n), "Wartezeit geeicht: 15 s +/- 1 %% (gemessen %.4f s)", sec);
    CHECK (n, sec > 14.85 && sec < 15.15);
    snprintf (n, sizeof (n), "kein Ueberlauf verpasst (%llu), LOAD %lu im 24-Bit-Bereich, ohne TICKINT",
              (unsigned long long) st_lost, (unsigned long) st.LOAD);
    CHECK (n, st_lost == 0 && ! st_badload && ! st_tickint);
    CHECK ("  Interrupts gesperrt, Takt aus dem RCC gelesen", irq_off && scu_calls >= 1);
}

int
main (void)
{
    static const uint32_t ziel_f[2] = { 72000000UL, 100000000UL };     /* F103 SetSysClockTo72; F411 HSE 25 MHz, PLL 25/200/2 */
    static const uint32_t ziel_vorgabe[2] = { 72000000UL, 168000000UL }; /* SystemCoreClock vor SystemCoreClockUpdate() in main() */
    static const uint32_t ziel_hsi[2] = { 8000000UL, 16000000UL };      /* vor SystemInit(): HSI */
    static const char * const ziel_name[2] = { "F103", "F411" };
    int     z, h = 0;

    printf ("[0] statisch: fault_reset() ohne Ausgabe, ohne neue Reload-Stelle, Takt aus dem RCC\n");
    CHECK ("kein log_-Aufruf in fault_reset()", FR_LOG_CALLS == 0);
    CHECK ("kein watchdog_reload() in fault_reset()", FR_WD_CALLS == 0);
    CHECK ("SystemCoreClockUpdate() in fault_reset()", FR_SCU_CALLS >= 1);
    CHECK ("genau ein NVIC_SystemReset() in fault_reset()", FR_RST_CALLS == 1);
    CHECK ("vier Handler rufen fault_reset() mit ihrem Namen", FR_HANDLERS == 4);

    for (z = 0; z < 2; z++)
    {
        FALL f[] =
        {
            { "im Betrieb, Ring leer",                         ziel_f[z],   ziel_f[z],       0,    1, 200, 0 },
            { "im Betrieb, Ring voll",                         ziel_f[z],   ziel_f[z],       TXLEN, 1, 200, 0 },
            { "im Betrieb, Ring halb, realistischer Pollschritt 5 Takte", ziel_f[z], ziel_f[z], TXLEN / 2, 1, 5, 0 },
            { "vor log_init()/delay_init(): UART und SysTick aus, SystemCoreClock noch Vorgabe", ziel_f[z], ziel_vorgabe[z], 0, 0, 200, 0 },
            { "vor SystemInit(): HSI, SystemCoreClock noch Vorgabe", ziel_hsi[z], ziel_vorgabe[z], 0, 0, 200, 0 },
            { "langsamer Pollschritt 2000 Takte",              ziel_f[z],   ziel_f[z],       0,    1, 2000, 0 },
        };
        int i;
        printf ("\n===== %s =====\n", ziel_name[z]);
        for (i = 0; i < (int) (sizeof (f) / sizeof (f[0])); i++) { f[i].handler = h; h = (h + 1) % 4; fall (&f[i]); }
    }

    printf ("\nWartezeit-Rechnung je Ziel (LOAD = SystemCoreClock / 1000 - 1, %lu Ueberlaeufe zu 1 ms):\n", (unsigned long) FAULT_RESET_WAIT_MS);
    for (z = 0; z < 2; z++)
    {
        uint32_t load = ziel_f[z] / 1000UL - 1UL;
        printf ("  %s  %9lu Hz  LOAD %6lu  (%lu Takte/ms)  %lu x 1 ms = %.3f s   [HSI vor SystemInit: %lu Hz, LOAD %lu]\n",
                ziel_name[z], (unsigned long) ziel_f[z], (unsigned long) load, (unsigned long) (load + 1),
                (unsigned long) FAULT_RESET_WAIT_MS, (double) FAULT_RESET_WAIT_MS * (load + 1) / ziel_f[z],
                (unsigned long) ziel_hsi[z], (unsigned long) (ziel_hsi[z] / 1000UL - 1UL));
    }

    printf ("\n%d Faelle, %d fehlgeschlagen\n", cases, fails);
    return fails ? 1 : 0;
}
