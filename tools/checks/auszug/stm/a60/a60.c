/* Pruefstand A60 (Paket 2026-10-11, S2.1): I2C-Fristen, woertlich aus i2c.c.
 *
 * Zeitmodell: Jede Abfrage von I2C_GetFlagStatus() kostet einen SysTick-Takt (5 us); ein Takt
 * zaehlt delay_counter wie SysTick_Handler() (delay.c) bis 0 herunter. delay_msec(n) (alter Stand)
 * verbraucht n * 200 Takte. Ein Stand, der nie zurueckkehrt, wird nach 2'000'000 Takten (10 s)
 * per longjmp abgebrochen und als "haengt" gewertet.
 *
 *   W  i2c_wait_for_flags: Flag nach k Takten (k = 0..1200), ein und zwei Flags; nie -> 0 nach 5 ms
 *   B  i2c_write / i2c_read: BUSY wird nach k Takten frei bzw. nie -> I2C_OK bzw. Fehler binnen Frist
 *   L  Lauf ohne Fehler: Daten gelesen, Rueckgabe I2C_OK */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <setjmp.h>
#ifdef PRUEFSTAND_SCHNELLE_TYPEN
#include "pruefstand.h"
#endif

#define DELAY_RESOLUTION_5_US   5
#define ENABLE  1
#define DISABLE 0
#define I2C_Direction_Transmitter 0
#define I2C_Direction_Receiver    1
enum { I2C_FLAG_SB = 1, I2C_FLAG_ADDR, I2C_FLAG_TXE, I2C_FLAG_BTF, I2C_FLAG_RXNE, I2C_FLAG_BUSY, I2C_FLAG_AF, NFLAG };
typedef struct { volatile uint32_t SR2; } I2C_MOCK;
static I2C_MOCK i2c_mock;
#define I2C_CHANNEL (&i2c_mock)

volatile uint32_t delay_counter;
static uint64_t   takt;
static jmp_buf    ausweg;
static uint64_t   bereit[NFLAG];          /* Flag gesetzt ab Takt (BUSY: FREI ab Takt); UINT64_MAX = nie */
static int        stops, resets;
static const uint64_t NIE = UINT64_MAX;

static void tick (void) { takt++; if (delay_counter > 0) delay_counter--; if (takt > 2000000) longjmp (ausweg, 1); }
/* Busmodell (M1, Nachpruefung): sda_tot = ein Slave haelt SDA low. Dann ist BUSY gesetzt und der Master
 * erzeugt KEIN START (SB kommt nie) -- RM0008. byte_haengt: der Slave bleibt mitten im Byte stehen (RXNE
 * kommt nie, ab da SDA low). stop_haengt: der Slave haelt SDA nach dem STOP der Transaktion fest. */
static int sda_tot, byte_haengt, stop_haengt, im_frist;
/* AF (G2-Korrektur): nack_adresse = der Slave quittiert das Adressbyte nicht -> ADDR kommt nie, AF gesetzt.
 * Der SWRST loescht SR1 und damit AF (I2C_SoftwareResetCmd unten). */
static int nack_adresse, af;
static int I2C_GetFlagStatus (I2C_MOCK * c, uint32_t f)
{
    (void) c; tick ();
    if (f == I2C_FLAG_RXNE && byte_haengt) { sda_tot = 1; return 0; }   /* bleibt aus, bis freigetaktet ist */
    if (sda_tot && f == I2C_FLAG_SB) return 0;
    if (f == I2C_FLAG_AF) return af;
    if (f == I2C_FLAG_ADDR && nack_adresse) return 0;
    if (f == I2C_FLAG_BUSY) return sda_tot || ! (takt >= bereit[f]);
    return takt >= bereit[f];
}
static void delay_msec (uint32_t ms) { uint32_t i; for (i = 0; i < ms * 200; i++) tick (); }
static void I2C_GenerateSTART (I2C_MOCK * c, int e) { (void) c; (void) e; }
static void I2C_GenerateSTOP (I2C_MOCK * c, int e) { (void) c; (void) e; stops++; if (stop_haengt) { sda_tot = 1; stop_haengt = 0; } }
static void I2C_SoftwareResetCmd (I2C_MOCK * c, int e) { (void) c; if (e) { resets++; af = 0; } }
static void I2C_DeInit (I2C_MOCK * c) { (void) c; }
static void i2c_init_i2c (void) { }
static void I2C_AcknowledgeConfig (I2C_MOCK * c, int e) { (void) c; (void) e; }
static int nack_bis_adresse = -1;          /* >= 0: nur diese Adresse antwortet, alle anderen NACKen (Adresssuche) */
static void I2C_Send7bitAddress (I2C_MOCK * c, uint8_t a, int d)
{
    (void) c; (void) d;
    if (nack_bis_adresse >= 0) nack_adresse = (a != nack_bis_adresse);
    if (nack_adresse) af = 1;
}
static void I2C_SendData (I2C_MOCK * c, uint8_t v) { (void) c; (void) v; }
/* Logzeile (A60 / AKI.3): genau "i2c: frist abgelaufen\r\n", gezaehlt; jeder andere Text ist ein Fehler */
static int logzeilen, logfremd, logcannot;
static void stub_log (const char * s)
{
    if (! strcmp (s, "i2c: frist abgelaufen\r\n")) logzeilen++;
    else if (! strcmp (s, "error: cannot reset I2C bus")) logcannot++;    /* bestehender Text aus i2c_reset_bus() */
    else logfremd++;
}
/* ---- GPIO/RCC-Attrappe fuer i2c_reset_bus()/i2c_init() (F103-Zweig) ----
 * Bus "haengt": BUSY bleibt gesetzt (bereit[BUSY] = NIE) und SDA liest low. heilbar = 1: Nach dem ersten
 * SCL-Takt gibt der Slave SDA frei, und sobald die Pins wieder im AF-Modus stehen, ist BUSY frei. */
#define STM32F10X
typedef struct { uint32_t GPIO_Pin, GPIO_Mode, GPIO_Speed; } GPIO_InitTypeDef;
#define GPIO_Mode_AF_OD 0x1C
#define GPIO_Speed_50MHz 3
#define GPIO_Speed_2MHz 2
#define SCL_PORT 1
#define SDA_PORT 2
#define SCL_PIN 0x40
#define SDA_PIN 0x80
#define PERIPH_SCL_SDA_PORT 8
#define RCC_PERIPH_I2C 0x200000
#define Bit_RESET 0
#define Bit_SET 1
static int heilbar, scl_takte, freitakten, pins_af = 1;
static void GPIO_SET_PIN_OUT_OD (int p, int n, int s) { (void) p; (void) n; (void) s; freitakten++; pins_af = 0; }
static void GPIO_SET_PIN_IN_NOPULL (int p, int n, int s) { (void) p; (void) n; (void) s; }
static void GPIO_SET_BIT (int p, int n) { if (p == SCL_PORT && n == SCL_PIN) { scl_takte++; if (heilbar) sda_tot = byte_haengt = 0; } }
static void GPIO_RESET_BIT (int p, int n) { (void) p; (void) n; }
static int  GPIO_ReadInputDataBit (int p, int n) { (void) p; (void) n; return sda_tot ? Bit_RESET : Bit_SET; }
static void RCC_APB2PeriphClockCmd (int x, int e) { (void) x; (void) e; }
static void RCC_APB1PeriphResetCmd (int x, int e) { (void) x; (void) e; }
static void RCC_APB1PeriphClockCmd (int x, int e) { (void) x; (void) e; }
static void GPIO_StructInit (GPIO_InitTypeDef * g) { memset (g, 0, sizeof *g); }
static void GPIO_Init (int p, GPIO_InitTypeDef * g)
{
    (void) p;
    if (g->GPIO_Mode == GPIO_Mode_AF_OD) pins_af = 1;
}
#define log_puts(s)     stub_log (s)
#define log_message(s)  stub_log (s)
static uint8_t lese_n;
static uint8_t I2C_ReceiveData (I2C_MOCK * c) { (void) c; return (uint8_t) (0xA0 + lese_n++); }

static uint_fast8_t eeprom_addr, eeprom_is_up;
#define EEPROM_FIRST_ADDR 0xA0
#include "auszug.c"
#ifndef I2C_ERROR_BUSY
#define I2C_ERROR_BUSY (-8)                       /* Stand ohne A60: nur damit die Gegenprobe uebersetzt */
#endif

static unsigned long faelle, fehl, kat_n[8], kat_f[8]; static const char * kat_name[8]; static int kat;
static void neue_kat (const char * n) { kat_name[++kat] = n; }
static void pruef (int ok, const char * was)
{ faelle++; kat_n[kat]++; if (! ok) { fehl++; kat_f[kat]++; if (kat_f[kat] <= 4 || (! strncmp (kat_name[kat], "M1", 2) || ! strncmp (kat_name[kat], "AF", 2))) printf ("  FEHL  [%s] %s\n", kat_name[kat], was); }   /* M1 ist klein: alles zeigen */ }
static void alle (uint64_t ab) { int i; for (i = 0; i < NFLAG; i++) bereit[i] = ab; bereit[I2C_FLAG_BUSY] = ab; }
static void start (void) { takt = 0; stops = resets = 0; delay_counter = 0; lese_n = 0; logzeilen = logfremd = logcannot = 0; freitakten = 0; scl_takte = 0; heilbar = 0; sda_tot = byte_haengt = stop_haengt = 0; nack_adresse = af = 0; }

int main (void)
{
    char was[160];
    uint64_t k;
    int r, cnt, haengt;

    i2c_init (100000);                         /* wie rtc_init()/eep_init(): Takt merken, Bus einmal freitakten */

    /* ---- W: ein Flag nach k Takten ---- */
    neue_kat ("W ein Flag");
    for (k = 0; k <= 1200; k++)
    {
        start (); alle (0); bereit[I2C_FLAG_TXE] = k;
        haengt = setjmp (ausweg); if (! haengt) r = i2c_wait_for_flags (I2C_FLAG_TXE, 0); else r = -1;
        if (k <= 990)
        {
            snprintf (was, sizeof was, "Flag nach %llu Takten: r=%d nach %llu Takten (Soll 1, hoechstens k+2)", (unsigned long long) k, r, (unsigned long long) takt);
            pruef (r == 1 && takt <= k + 2 && stops == 0 && logzeilen == 0 && ! logfremd, was);
        }
        else if (k >= 1010)
        {
            snprintf (was, sizeof was, "Flag nach %llu Takten (nach der Frist): r=%d nach %llu Takten (Soll 0 nach 995..1005)", (unsigned long long) k, r, (unsigned long long) takt);
            pruef (r == 0 && takt >= 995 && takt <= 1005 && stops == 1 && resets == 1, was);
            pruef (logzeilen == 1 && ! logfremd, "Fristablauf (ein Flag): nicht genau eine Zeile 'i2c: frist abgelaufen'");
            pruef (freitakten == 0, "Flag-Frist taktet den Bus frei (soll nur die BUSY-Frist)");
        }
    }
    start (); alle (0); bereit[I2C_FLAG_SB] = NIE;
    haengt = setjmp (ausweg); if (! haengt) r = i2c_wait_for_flags (I2C_FLAG_SB, 0); else r = -1;
    snprintf (was, sizeof was, "Flag nie: r=%d nach %llu Takten (Soll 0 nach rund 1000 = 5 ms, STOP und Reset)", r, (unsigned long long) takt);
    pruef (r == 0 && takt >= 995 && takt <= 1005 && stops == 1 && resets == 1, was);
    pruef (logzeilen == 1 && ! logfremd, "Flag nie: nicht genau eine Zeile 'i2c: frist abgelaufen'");

    /* ---- W: zwei Flags ---- */
    neue_kat ("W zwei Flags");
    for (k = 0; k <= 1200; k += 7)
    {
        uint64_t k2;
        for (k2 = 0; k2 <= 1200; k2 += 113)
        {
            uint64_t sp = k > k2 ? k : k2;
            start (); alle (0); bereit[I2C_FLAG_TXE] = k; bereit[I2C_FLAG_BTF] = k2;
            haengt = setjmp (ausweg); if (! haengt) r = i2c_wait_for_flags (I2C_FLAG_TXE, I2C_FLAG_BTF); else r = -1;
            snprintf (was, sizeof was, "TXE nach %llu, BTF nach %llu: r=%d nach %llu Takten", (unsigned long long) k, (unsigned long long) k2, r, (unsigned long long) takt);
            if (sp <= 990)       pruef (r == 1 && takt <= 2 * sp + 4 && logzeilen == 0, was);
            else if (sp >= 1010) pruef (r == 0 && takt >= 995 && takt <= 1006 && logzeilen == 1 && ! logfremd, was);
        }
    }

    /* ---- B: BUSY in i2c_write und i2c_read ---- */
    neue_kat ("B BUSY");
    for (k = 0; k <= 1200; k += 3)
    {
        for (cnt = 1; cnt <= 3; cnt += 2)
        {
            int art;
            for (art = 0; art < 2; art++)            /* 0 = write, 1 = read */
            {
                uint8_t buf[4] = { 1, 2, 3, 4 };
                start (); alle (0); bereit[I2C_FLAG_BUSY] = k;
                haengt = setjmp (ausweg);
                if (! haengt) r = art ? i2c_read (0x50, 0x10, 1, buf, cnt) : i2c_write (0x50, 0x10, 1, buf, cnt); else r = 99;
                snprintf (was, sizeof was, "%s n=%d, BUSY frei nach %llu Takten: r=%d nach %llu Takten%s",
                          art ? "i2c_read" : "i2c_write", cnt, (unsigned long long) k, r, (unsigned long long) takt, haengt ? " -- HAENGT" : "");
                if (k <= 990)       pruef (r == 0 && takt <= k + 40 && logzeilen == 0, was);
                else if (k >= 1040) pruef (r < 0 && r != 99 && takt <= 1040 + 4200 && stops >= 2 && resets == 1 && logzeilen == 1 && ! logfremd && freitakten == 1 && pins_af, was);
            }
        }
    }
    {
        int art; uint8_t buf[4];
        for (art = 0; art < 2; art++)
        {
            start (); alle (0); bereit[I2C_FLAG_BUSY] = NIE;
            haengt = setjmp (ausweg);
            if (! haengt) r = art ? i2c_read (0x50, 0x10, 1, buf, 2) : i2c_write (0x50, 0x10, 1, buf, 2); else r = 99;
            snprintf (was, sizeof was, "%s, BUSY nie frei: r=%d nach %llu Takten%s (Soll Fehler binnen 5 ms)",
                      art ? "i2c_read" : "i2c_write", r, (unsigned long long) takt, haengt ? " -- HAENGT (alter Stand: L360)" : "");
            if (haengt || ! (r < 0 && takt <= 1040 + 4200)) printf ("  FEHL  [B BUSY] %s\n", was);   /* immer sichtbar, nicht nur die ersten vier */
            pruef (! haengt && r < 0 && takt <= 1040 + 4200, was);
            pruef (logzeilen == 1 && ! logfremd, "BUSY nie frei: nicht genau eine Zeile 'i2c: frist abgelaufen'");
#ifdef HAVE_NOT_BUSY
            pruef (r == I2C_ERROR_BUSY, "Fehlercode bei BUSY nicht I2C_ERROR_BUSY");
#else
            pruef (0, "kein I2C_ERROR_BUSY (Stand ohne A60)");
#endif
        }
    }

    /* ---- L: Lauf ohne Fehler ---- */
    neue_kat ("L ohne Fehler");
    {
        uint8_t buf[8];
        start (); alle (3); memset (buf, 0, sizeof buf);
        haengt = setjmp (ausweg); if (! haengt) r = i2c_read (0x50, 0x10, 1, buf, 4); else r = 99;
        pruef (r == I2C_OK && buf[0] == 0xA0 && buf[3] == 0xA3, "i2c_read 4 Byte: falsche Daten oder Fehler");
        start (); alle (3);
        haengt = setjmp (ausweg); if (! haengt) r = i2c_write (0x50, 0x10, 1, buf, 4); else r = 99;
        pruef (r == I2C_OK && logzeilen == 0, "i2c_write 4 Byte: Fehler oder Logzeile");
        snprintf (was, sizeof was, "i2c_write 4 Byte ohne Wartezeit dauert %llu Takte (Soll unter 100, alter Stand: je Flag 1 ms)", (unsigned long long) takt);
        pruef (takt < 100, was);
    }

    /* ---- M1: Freitakten nach JEDER Frist, wenn der Bus danach belegt ist (Review S2, Nachpruefung) ---- */
    neue_kat ("M1 Freitakten");
    {
        int art, z, fall; uint8_t buf[4] = { 1, 2, 3, 4 };
        #define ZUGRIFF(a) (setjmp (ausweg) ? 99 : ((a) ? i2c_read (0x50, 0x10, 1, buf, 2) : i2c_write (0x50, 0x10, 1, buf, 2)))
        #define NULLEN() do { takt = 0; logzeilen = logfremd = logcannot = 0; freitakten = 0; stops = resets = 0; } while (0)
        for (art = 0; art < 2; art++)
        {
            const char * wer = art ? "i2c_read" : "i2c_write";
            for (fall = 0; fall < 2; fall++)                 /* 0 = Haenger beim STOP, 1 = Haenger mitten im Byte */
            {
                if (fall == 1 && ! art) continue;             /* mitten im Byte: nur beim Lesen (RXNE) */
                start (); alle (0); heilbar = 1;
                if (fall == 0) stop_haengt = 1; else byte_haengt = 1;
                r = ZUGRIFF (art);
                snprintf (was, sizeof was, "%s, Haenger %s (heilbar): r=%d, %llu Takte, Fristzeilen %d, Freitakten %d, Pins AF %d, SDA %s",
                          wer, fall ? "mitten im Byte" : "beim STOP", r, (unsigned long long) takt, logzeilen, freitakten, pins_af, sda_tot ? "low" : "frei");
                pruef (r == (fall ? I2C_ERROR_NO_FLAG_RXNE : I2C_ERROR_BUSY) && logzeilen == 1 && freitakten == 1 && pins_af && ! sda_tot && ! logcannot && takt <= 1040 + 4200, was);
                NULLEN ();
                r = ZUGRIFF (art);
                snprintf (was, sizeof was, "%s, Haenger %s: naechster Zugriff r=%d, Fristzeilen %d, Freitakten %d",
                          wer, fall ? "mitten im Byte" : "beim STOP", r, logzeilen, freitakten);
                pruef (r == I2C_OK && logzeilen == 0 && freitakten == 0, was);
            }

            /* SDA dauerhaft tot: jeder Zugriff scheitert am START (SB), taktet frei, bleibt begrenzt */
            start (); alle (0); heilbar = 0; sda_tot = 1;
            for (z = 0; z < 5; z++)
            {
                NULLEN ();
                r = ZUGRIFF (art);
                snprintf (was, sizeof was, "%s, SDA dauerhaft low, Zugriff %d: r=%d, %llu Takte, Fristzeilen %d, Freitakten %d, 'cannot reset' %d",
                          wer, z + 1, r, (unsigned long long) takt, logzeilen, freitakten, logcannot);
                pruef (r == I2C_ERROR_NO_FLAG_SB && takt <= 1040 + 4200 && logzeilen == 1 && freitakten == 1 && logcannot == 1 && pins_af && ! logfremd, was);
            }
        }

        /* Flag-Frist OHNE belegten Bus (Slave antwortet nicht, ADDR kommt nie): kein Freitakten, keine 20 ms */
        for (art = 0; art < 2; art++)
        {
            start (); alle (0); bereit[I2C_FLAG_ADDR] = NIE;
            r = ZUGRIFF (art);
            snprintf (was, sizeof was, "%s, ADDR nie, Bus frei: r=%d, %llu Takte, Fristzeilen %d, Freitakten %d",
                      art ? "i2c_read" : "i2c_write", r, (unsigned long long) takt, logzeilen, freitakten);
            pruef (r == I2C_ERROR_NO_FLAG_ADDR && logzeilen == 1 && freitakten == 0 && takt <= 1010, was);
        }
    }

    /* ---- G2-Korrektur und Review S3.13 M2 (A73): NACK ----
     * Still NUR waehrend der Adresssuche in eeprom_init (i2c_nack_quiet); im Betrieb genau eine Zeile.
     * In beiden Faellen KEIN Freitakten: Ein NACK ist kein haengender Bus. */
    neue_kat ("AF NACK");
    {
        int art, i; uint8_t buf[4] = { 1, 2, 3, 4 };
        for (art = 0; art < 2; art++)
        {
            const char * wer = art ? "i2c_read" : "i2c_write";
            start (); alle (0); nack_adresse = 1; i2c_nack_quiet = 0;
            r = ZUGRIFF (art);
            snprintf (was, sizeof was, "%s, NACK im Betrieb: r=%d, Fristzeilen %d, Freitakten %d, STOP %d, Reset %d, AF danach %d",
                      wer, r, logzeilen, freitakten, stops, resets, af);
            pruef (r == I2C_ERROR_NO_FLAG_ADDR && logzeilen == 1 && freitakten == 0 && stops >= 1 && resets == 1 && af == 0 && takt <= 1010, was);

            /* altes AF bleibt nicht stehen: danach ein echter Fristablauf (Bus haengt beim START) -> genau eine Zeile, Freitakten */
            NULLEN (); nack_adresse = 0; sda_tot = 1; heilbar = 1;
            r = ZUGRIFF (art);
            snprintf (was, sizeof was, "%s, nach einem NACK echter Fristablauf (SB): r=%d, Fristzeilen %d, Freitakten %d", wer, r, logzeilen, freitakten);
            pruef (r == I2C_ERROR_NO_FLAG_SB && logzeilen == 1 && freitakten == 1, was);
        }
        /* die ECHTE Suche aus eeprom_init(): 7 x NACK, die 8. Adresse antwortet -> keine Zeile, kein Freitakten */
        start (); alle (0); eeprom_is_up = 0; i2c_nack_quiet = 0;
        nack_bis_adresse = 0xAE;
        eep_suche ();
        nack_bis_adresse = -1;
        snprintf (was, sizeof was, "eeprom_init-Suche, 7 x NACK, 8. antwortet: EEPROM up %d, Fristzeilen %d, Freitakten %d, Flag danach %d",
                  eeprom_is_up, logzeilen, freitakten, i2c_nack_quiet);
        pruef (eeprom_is_up == 1 && logzeilen == 0 && freitakten == 0, was);
        pruef (i2c_nack_quiet == 0, "nach der Suche: i2c_nack_quiet nicht zurueckgesetzt");
        /* und danach im Betrieb wieder eine Zeile */
        NULLEN (); nack_adresse = 1; r = ZUGRIFF (0);
        pruef (logzeilen == 1 && freitakten == 0, "NACK nach der Suche (Betrieb): nicht genau eine Zeile");
    }

    printf ("  Kategorien:\n");
    for (r = 1; r <= kat; r++) printf ("    %-16s %6lu Pruefungen, %lu FEHL\n", kat_name[r], kat_n[r], kat_f[r]);
    printf ("a60: %lu Pruefungen, %lu FEHL -- %s\n", faelle, fehl, fehl ? "FEHL" : "OK");
    return fehl ? 1 : 0;
}
