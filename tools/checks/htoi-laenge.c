/* Pruefstand fuer die Laengenpruefung der STM-Kommandozeile (A41 / L265).
 *
 * WARUM ES DIESE DATEI GIBT
 *
 * Im STM folgt auf jedes htoi (x, n) ein UNBEDINGTES x += n -- gezaehlt am 05.10.2026
 * ueber den ganzen Baum: main.c 39, tables.c 22, esp-spiffs.c 5, tftled.c 1, zusammen 67
 * Vorrueckungen. Stand der Zeiger schon auf dem Terminator, zeigt er danach DAHINTER, und
 * der naechste htoi liest dort. Die Korrektur an htoi() (L263) begrenzt den Schaden, hebt
 * ihn nicht auf: Hinter dem Terminator steht nicht zwingend eine weitere Null.
 *
 * Am Geraet laesst sich das nicht ausloesen, ohne die Uhr zu gefaehrden. Deshalb hier,
 * mit einer SCHUTZSEITE: Die Kommandozeile liegt so im Speicher, dass ihr Terminator das
 * LETZTE lesbare Byte vor einer PROT_NONE-Seite ist. Jeder Lesezugriff hinter dem
 * Terminator loest damit SIGSEGV/SIGBUS aus -- der Befund wird sichtbar statt
 * wahrscheinlich.
 *
 * DIE ERWARTUNGEN STAMMEN NICHT AUS DEM GEPRUEFTEN CODE.
 * Die Mindestlaengen unten sind aus den SENDEFORMATEN des ESP abgeleitet
 * (ESP8266/ESP-uclock/vars.cpp, die Serial.printf ("CMD ...")-Zeilen), also aus der
 * Gegenseite der Bruecke -- nicht aus main.c. Wer seine Vektoren aus der Umsetzung
 * ableitet, prueft nur, ob sie mit sich selbst uebereinstimmt.
 *
 * Typangleichung nach L256 ueber pruefstand.h: Der STM32 ist 32-bittig.
 *
 * WAS DIESER PRUEFSTAND NICHT VON ALLEIN LEISTET, und wie die Luecke geschlossen wurde:
 * Er prueft ein MODELL der Firmware, nicht die Firmware. Die Tabelle unten und die in
 * src/main.c koennten auseinanderlaufen, ohne dass hier etwas auffiele. Gegengerechnet
 * wurde deshalb einmal direkt: esp8266_cmd_min_len() aus src/main.c herausgeloest und
 * gegen min_len() hier gestellt, ueber alle 127 x 128 = 16'256 Kombinationen aus erstem
 * und zweitem Byte. Null Abweichungen (05.10.2026). Wer die Tabelle anfasst, wiederholt
 * das -- sonst prueft diese Datei nur noch sich selbst.
 *
 * EINMAL FEHLGESCHLAGEN (DIR-014), und zwar dreimal echt statt gestellt:
 *   1. Erster Lauf: "0 Zeilen verworfen" bei 14 verworfenen -- der Zaehler stand im
 *      Kindprozess und erreichte den Elternprozess nie. Genau die Gattung aus L181/L185.
 *   2. Derselbe Lauf meldete neun Fehler, die keine waren: Die Erwartung "alt faellt"
 *      traf fuer Zeilen mit nur einem fehlenden Feld nicht zu -- ein Befund ueber die
 *      Reichweite von L263, nicht ueber den Code. Daher die dritte Erwartungsklasse.
 *   3. Gegenprobe mit absichtlich zu kleiner Tabelle (N auf 3, T auf 5): drei Fehler,
 *      Exit 1. Ohne diesen Lauf waere nur belegt, dass er bei richtigem Code schweigt.
 *
 *   cc -O1 -I tools/checks -o /tmp/htoi-laenge tools/checks/htoi-laenge.c && /tmp/htoi-laenge
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include "pruefstand.h"

#define ESP8266_MAX_CMD_LEN     127                 /* wie src/esp8266/esp8266.h */
#define CMD_BUF_SIZE            (ESP8266_MAX_CMD_LEN + 1)

/*--------------------------------------------------------------------------------------
 * htoi () -- Kopie aus src/base/base.c, Stand nach L263 (prueft buf[i], nicht *buf)
 *------------------------------------------------------------------------------------*/
static uint16_t
htoi (const char * buf, uint8_t max_digits)
{
    uint8_t     i;
    uint8_t     x;
    uint16_t    sum = 0;

    for (i = 0; i < max_digits && buf[i]; i++)
    {
        x = buf[i];
        if      (x >= '0' && x <= '9') x -= '0';
        else if (x >= 'A' && x <= 'F') x -= 'A' - 10;
        else if (x >= 'a' && x <= 'f') x -= 'a' - 10;
        else                           x = 0;
        sum <<= 4;
        sum += x;
    }
    return sum;
}

/*--------------------------------------------------------------------------------------
 * Die Mindestlaengen -- EINSCHLIESSLICH Kommandobuchstabe, 0 = unbekannt.
 *
 * Quelle: die Sendeformate des ESP in ESP8266/ESP-uclock/vars.cpp:
 *   R%02x                           -> 3     N%02x%02x%02x              -> 7
 *   n%02x%02x%02x                   -> 7     S%02x%s                    -> 3
 *   T%02x%04d%02d%02d%02d%02d%02d   -> 17    DC%02x%02x%02x%02x[%02x]   -> 10
 *   xN%02x%s (A C M D)              -> 4     xD/xE/xF%02x%02x           -> 6
 *   OT/OI/OD/OC/OY/OF%02x%02x       -> 6     OS%02x%04x                 -> 8
 *   ON%02x%s                        -> 4     t/a/l%02x%02x%02x%02x      -> 9
 *   I%02x%02x%04x%04x               -> 13    GTs / GSs                  -> 3
 *
 * DC bewusst 10 und nicht 12: Ob der ESP das vierte Byte (Weiss) mitschickt, haengt an
 * seinem use_rgbw -- einem Wert, den ihm DIESELBE Bruecke geliefert hat (vars.cpp,
 * set_dsp_color_var). Eine Pruefung auf 12 wuerde nach einem verlorenen
 * use_rgbw-Kommando gueltige Farbzeilen abweisen. Dieselbe Falle hat die ESP-Seite
 * benannt (A37).
 *------------------------------------------------------------------------------------*/
static unsigned
min_len (const char * line)
{
    switch (line[0])
    {
        case 'R': return  3;
        case 'N': return  7;
        case 'n': return  7;
        case 'S': return  3;
        case 'T': return 17;
        case 't': return  9;
        case 'a': return  9;
        case 'l': return  9;
        case 'I': return 13;
        case 'G': return  3;

        case 'D':
            return (line[1] == 'C') ? 10 : 4;

        case 'A':
        case 'C':
        case 'M':
            switch (line[1])
            {
                case 'D': case 'E': case 'F': return 6;
                case 'N':                     return 4;
            }
            return 4;

        case 'O':
            switch (line[1])
            {
                case 'S':                                         return 8;
                case 'T': case 'I': case 'D': case 'C':
                case 'Y': case 'F':                               return 6;
                case 'N':                                         return 4;
            }
            return 4;
    }
    return 0;                                   /* unbekannt -- MUSS durchgelassen werden (L269) */
}

/*--------------------------------------------------------------------------------------
 * Der Zerlegevorgang, so wie ihn main.c fuehrt: lesen, dann UNBEDINGT vorruecken.
 * Rueckgabe: eine Quersumme aller gelesenen Werte, damit "alt" und "neu" auf gueltigen
 * Eingaben byte-identisch vergleichbar sind.
 *------------------------------------------------------------------------------------*/
static uint32_t
zerlegen (const char * line)
{
    const char *    p = line;
    uint32_t        acc = 0;
    char            c   = *p++;
    int             i;

    switch (c)
    {
        case 'R':
            acc += htoi (p, 2); p += 2;
            break;

        case 'N':
        case 'n':
            acc += htoi (p, 2); p += 2;
            acc += htoi (p, 2); p += 2;
            acc += htoi (p, 2); p += 2;
            break;

        case 'S':
            acc += htoi (p, 2); p += 2;
            acc += (uint32_t) strlen (p);                    /* Text, darf leer sein */
            break;

        case 'T':
            acc += htoi (p, 2); p += 2;
            for (i = 0; i < 14; i++)                         /* direkt indiziert, main.c */
            {
                acc += (uint32_t) (unsigned char) p[i];
            }
            break;

        case 't':
        case 'a':
        case 'l':
            acc += htoi (p, 2); p += 2;
            acc += htoi (p, 2) + (uint32_t) (htoi (p + 2, 2) << 8); p += 4;
            acc += htoi (p, 2); p += 2;
            break;

        case 'D':
        {
            char sub = *p++;
            acc += htoi (p, 2); p += 2;
            if (sub == 'C')
            {
                acc += htoi (p, 2); p += 2;
                acc += htoi (p, 2); p += 2;
                acc += htoi (p, 2); p += 2;
                acc += htoi (p, 2); p += 2;                  /* Weiss, nur im RGBW-Bau */
            }
            else
            {
                acc += (uint32_t) strlen (p);
            }
            break;
        }

        case 'A':
        case 'C':
        case 'M':
        {
            char sub = *p++;
            acc += htoi (p, 2); p += 2;
            if (sub == 'D' || sub == 'F')
            {
                acc += htoi (p, 2); p += 2;
            }
            else if (sub == 'N')
            {
                acc += (uint32_t) strlen (p);
            }
            break;
        }

        case 'O':
        {
            char sub = *p++;
            acc += htoi (p, 2); p += 2;
            if (sub == 'S')
            {
                acc += htoi (p, 4);
            }
            else if (sub == 'N')
            {
                acc += (uint32_t) strlen (p);
            }
            else
            {
                acc += htoi (p, 2); p += 2;
            }
            break;
        }

        case 'I':
            for (i = 0; i < 12; i++)
            {
                char x = p[i];
                acc += (uint32_t) (unsigned char) x;
                if (! x) break;
            }
            break;

        case 'G':
            p++;
            acc += (uint32_t) (unsigned char) *p;
            break;

        default:
            break;                                           /* unbekannt: nichts zu tun */
    }
    return acc;
}

/* ALT: ohne jede Laengenpruefung -- der Stand vor A41. */
static uint32_t alt (const char * line) { return zerlegen (line); }

/* NEU: EINE Laengenpruefung vorn, Grenze aus der Puffergroesse abgeleitet. */
static unsigned long neu_verworfen = 0;
static uint32_t
neu (const char * line)
{
    unsigned n = 0;
    unsigned want;

    while (n < CMD_BUF_SIZE && line[n])                      /* begrenzt, nicht strlen() */
    {
        n++;
    }

    if (n == 0 || n == CMD_BUF_SIZE)                         /* leer bzw. ohne Terminator */
    {
        neu_verworfen++;
        return 0;
    }

    want = min_len (line);

    if (n < want)
    {
        neu_verworfen++;
        return 0;
    }
    return zerlegen (line);
}

/*--------------------------------------------------------------------------------------
 * tftled_layout_get_line () -- die exponierte Stelle in src/tftled/tftled.c
 *------------------------------------------------------------------------------------*/
static uint32_t
tftled_alt (const char * str)
{
    uint32_t acc = htoi (str, 2);
    str += 2;
    while (*str) { acc += (unsigned char) *str; str++; }
    return acc;
}

static uint32_t
tftled_neu (const char * str)
{
    unsigned n = 0;
    uint32_t acc;

    while (n < 25 && str[n]) n++;                            /* 25 = sizeof (esp8266.u.disp) */
    if (n < 2) { neu_verworfen++; return 0; }

    acc = htoi (str, 2);
    str += 2; n -= 2;
    while (n > 0 && *str) { acc += (unsigned char) *str; str++; n--; }
    return acc;
}

/*--------------------------------------------------------------------------------------
 * esp_diffs_read_icon () -- die exponierte Stelle in src/esp-spiffs/esp-spiffs.c
 *------------------------------------------------------------------------------------*/
static uint32_t
icon_alt (const char * p)
{
    uint32_t acc = 0;
    while (*p) { acc += htoi (p, 2); p += 2; }
    return acc;
}

static uint32_t
icon_neu (const char * p, const char * end)
{
    uint32_t acc = 0;
    while (p + 1 < end && p[0] && p[1]) { acc += htoi (p, 2); p += 2; }
    return acc;
}

/*--------------------------------------------------------------------------------------
 * Schutzseite: die Zeile endet buendig an der Seitengrenze, dahinter PROT_NONE.
 *------------------------------------------------------------------------------------*/
static char *   seite;
static long     seitengroesse;

static void
seite_anlegen (void)
{
    seitengroesse = sysconf (_SC_PAGESIZE);
    seite = mmap (NULL, 2 * seitengroesse, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (seite == MAP_FAILED) { perror ("mmap"); exit (2); }
    if (mprotect (seite + seitengroesse, seitengroesse, PROT_NONE) != 0)
    {
        perror ("mprotect"); exit (2);
    }
}

/* Legt s so ab, dass sein Terminator das letzte lesbare Byte ist. */
static char *
buendig (const char * s)
{
    size_t  len = strlen (s);
    char *  ziel = seite + seitengroesse - (len + 1);
    memcpy (ziel, s, len + 1);
    return ziel;
}

/*--------------------------------------------------------------------------------------
 * Ein Fall im Kindprozess: faellt er, sehen wir das Signal statt eines Absturzes hier.
 *------------------------------------------------------------------------------------*/
typedef enum { F_CMD, F_TFTLED, F_ICON } FORM;

/* Was ueber die Pipe geht. Der Zaehler steht BEWUSST hier und nicht als globale
 * Variable des Elternprozesses: Der Fall laeuft im Kind, und eine Zaehlung, die den
 * Empfaenger nicht erreicht, ist keine (DIR-014 -- genau diese Gattung ist in diesem
 * Projekt fuenfmal aufgetreten). Der erste Lauf dieses Pruefstands meldete
 * "0 Zeilen verworfen", obwohl er 14 verworfen hatte. */
typedef struct
{
    uint32_t        wert;
    uint32_t        verworfen;
} ERGEBNIS;

static int
lauf (FORM form, int neue_fassung, const char * s, ERGEBNIS * erg)
{
    int     fd[2];
    pid_t   pid;
    int     status;

    if (pipe (fd) != 0) { perror ("pipe"); exit (2); }

    pid = fork ();
    if (pid == 0)
    {
        char *      z = buendig (s);
        ERGEBNIS    r;

        neu_verworfen = 0;
        switch (form)
        {
            case F_CMD:    r.wert = neue_fassung ? neu (z)        : alt (z);        break;
            case F_TFTLED: r.wert = neue_fassung ? tftled_neu (z) : tftled_alt (z); break;
            case F_ICON:   r.wert = neue_fassung ? icon_neu (z, seite + seitengroesse) : icon_alt (z); break;
            default:       r.wert = 0;                                              break;
        }
        r.verworfen = (uint32_t) neu_verworfen;
        if (write (fd[1], &r, sizeof (r)) != (ssize_t) sizeof (r)) { _exit (3); }
        _exit (0);
    }

    close (fd[1]);
    erg->wert = 0;
    erg->verworfen = 0;
    if (read (fd[0], erg, sizeof (*erg)) != (ssize_t) sizeof (*erg))
    {
        erg->wert = 0;
        erg->verworfen = 0;
    }
    close (fd[0]);
    waitpid (pid, &status, 0);

    if (WIFSIGNALED (status)) return WTERMSIG (status);
    return 0;
}

static int  faelle = 0, fehler = 0;
static unsigned long verworfen_gesamt = 0;

/* Drei Erwartungen, nicht zwei. Der erste Entwurf kannte nur "alt faellt" und
 * "alt == neu" und meldete daraufhin neun Fehler, die keine waren:
 *
 *   Seit der htoi()-Korrektur aus L263 haelt htoi am ersten Nullbyte. Eine Zeile, der
 *   GENAU EIN Feld fehlt, laeuft deshalb schon heute glimpflich ab -- der Zeiger landet
 *   auf dem Terminator, htoi liest ihn, bricht ab, und das letzte unbedingte "+= 2"
 *   zeigt zwar dahinter, aber niemand liest dort noch.
 *
 *   Gefaehrlich wird es erst, wenn der Zeiger den Terminator UEBERSPRINGT: wenn
 *   mindestens zwei Felder fehlen, oder wenn ein Vorruecken breiter ist als das
 *   vorangegangene Lesen (die 14 direkt indizierten Ziffern bei T, das "+= 4" bei
 *   t/a/l), oder wenn eine Schleife paarweise laeuft und die Laenge ungerade ist
 *   (esp-spiffs).
 *
 * Diese Unterscheidung ist ein Befund und kein Pruefstandsdetail: Sie sagt, WIE WEIT
 * L263 schon getragen hat und was A41 zusaetzlich deckt. */
typedef enum
{
    ERW_GEFAHR,         /* die alte Fassung MUSS hinter den Terminator lesen */
    ERW_GLEICH,         /* gueltige Zeile: alt und neu muessen gleich rechnen */
    ERW_VERWORFEN,      /* zu kurz, aber heute (noch) ohne Zugriffsfehler -- neu verwirft */
    ERW_GEFAHR_BEGRENZT /* alt faellt; neu VERWIRFT NICHT, sondern begrenzt die Schleife */
} ERWARTUNG;

static void
pruefe (FORM form, const char * name, const char * s, ERWARTUNG erw)
{
    ERGEBNIS    ra, rn;
    int         sa, sn;
    const char *formname = (form == F_CMD) ? "cmd" : (form == F_TFTLED) ? "disp" : "icon";
    const char *urteil = "ok";

    faelle++;
    sa = lauf (form, 0, s, &ra);
    sn = lauf (form, 1, s, &rn);
    verworfen_gesamt += rn.verworfen;

    if (sn != 0)
    {
        urteil = "FEHLER: die neue Fassung ist gefallen";
        fehler++;
    }
    else if ((erw == ERW_GEFAHR || erw == ERW_GEFAHR_BEGRENZT) && sa == 0)
    {
        urteil = "FEHLER: alt faellt nicht -- der Fall trifft den Befund nicht";
        fehler++;
    }
    else if (erw == ERW_GLEICH && (sa != 0 || ra.wert != rn.wert))
    {
        urteil = "FEHLER: gueltige Eingabe, aber alt != neu";
        fehler++;
    }
    else if (erw == ERW_VERWORFEN && rn.verworfen == 0)
    {
        urteil = "FEHLER: neu haette die Zeile verwerfen muessen";
        fehler++;
    }
    else if (erw == ERW_GEFAHR && rn.verworfen == 0)
    {
        urteil = "FEHLER: neu ist zwar nicht gefallen, hat aber nichts verworfen";
        fehler++;
    }

    printf ("  %-5s %-30s alt=%-8s neu=%-8s verw=%u  %s\n",
            formname, name,
            sa ? (sa == SIGSEGV ? "SIGSEGV" : sa == SIGBUS ? "SIGBUS" : "Signal") : "ok",
            sn ? (sn == SIGSEGV ? "SIGSEGV" : sn == SIGBUS ? "SIGBUS" : "Signal") : "ok",
            rn.verworfen, urteil);
}

int
main (void)
{
    seite_anlegen ();

    printf ("Pruefstand A41 / L265 -- Laengenpruefung der STM-Kommandozeile\n");
    printf ("Schutzseite: der Terminator ist das letzte lesbare Byte, dahinter PROT_NONE.\n\n");

    printf ("1. Zu kurze Zeilen, bei denen der Zeiger den Terminator UEBERSPRINGT\n");
    printf ("   -- die alte Fassung muss hier hinter den Terminator lesen\n");
    pruefe (F_CMD, "N nur Index",          "N00",               ERW_GEFAHR);
    pruefe (F_CMD, "N nur Buchstabe",      "N",                 ERW_GEFAHR);
    pruefe (F_CMD, "n nur Index",          "n00",               ERW_GEFAHR);
    pruefe (F_CMD, "T halbes Datum",       "T0020261005",       ERW_GEFAHR);
    pruefe (F_CMD, "T nur Index",          "T00",               ERW_GEFAHR);
    pruefe (F_CMD, "t halbe Minuten",      "t0001",             ERW_GEFAHR);
    pruefe (F_CMD, "a halbe Minuten",      "a0001",             ERW_GEFAHR);
    pruefe (F_CMD, "l nur Index",          "l00",               ERW_GEFAHR);
    pruefe (F_CMD, "DC nur Index",         "DC00",              ERW_GEFAHR);
    pruefe (F_CMD, "DC ohne blau",         "DC000102",          ERW_GEFAHR);

    printf ("\n2. Zu kurze Zeilen, die L263 schon abfaengt -- neu verwirft sie trotzdem\n");
    pruefe (F_CMD, "N ohne hi",            "N0012",             ERW_VERWORFEN);
    pruefe (F_CMD, "n ohne Wert",          "n0001",             ERW_VERWORFEN);
    pruefe (F_CMD, "R ohne Index",         "R",                 ERW_VERWORFEN);
    pruefe (F_CMD, "t ohne Flags",         "t000102",           ERW_VERWORFEN);
    pruefe (F_CMD, "AD ohne Wert",         "AD00",              ERW_VERWORFEN);
    pruefe (F_CMD, "OT ohne Typ",          "OT00",              ERW_VERWORFEN);
    pruefe (F_CMD, "OS halbes Datum",      "OS0012",            ERW_VERWORFEN);
    pruefe (F_CMD, "I zu kurz",            "I0001",             ERW_VERWORFEN);
    pruefe (F_CMD, "leere Zeile",          "",                  ERW_VERWORFEN);

    printf ("\n3. Gueltige Zeilen -- alt und neu muessen GLEICH rechnen\n");
    pruefe (F_CMD, "N vollstaendig",       "N000102",           ERW_GLEICH);
    pruefe (F_CMD, "n vollstaendig",       "n000102",           ERW_GLEICH);
    pruefe (F_CMD, "R vollstaendig",       "R07",               ERW_GLEICH);
    pruefe (F_CMD, "S mit Text",           "S03meinhost",       ERW_GLEICH);
    pruefe (F_CMD, "S ohne Text",          "S03",               ERW_GLEICH);
    pruefe (F_CMD, "T vollstaendig",       "T0020261005213000", ERW_GLEICH);
    pruefe (F_CMD, "t vollstaendig",       "t00010203",         ERW_GLEICH);
    pruefe (F_CMD, "a vollstaendig",       "a00010203",         ERW_GLEICH);
    pruefe (F_CMD, "l vollstaendig",       "l00010203",         ERW_GLEICH);
    pruefe (F_CMD, "DC ohne Weiss",        "DC00112233",        ERW_GLEICH);
    pruefe (F_CMD, "DC mit Weiss",         "DC0011223344",      ERW_GLEICH);
    pruefe (F_CMD, "DN mit Name",          "DN00Wortuhr",       ERW_GLEICH);
    pruefe (F_CMD, "DN ohne Name",         "DN00",              ERW_GLEICH);
    pruefe (F_CMD, "AD vollstaendig",      "AD0005",            ERW_GLEICH);
    pruefe (F_CMD, "AE vollstaendig",      "AE0005",            ERW_GLEICH);
    pruefe (F_CMD, "AF vollstaendig",      "AF0003",            ERW_GLEICH);
    pruefe (F_CMD, "CD vollstaendig",      "CD0005",            ERW_GLEICH);
    pruefe (F_CMD, "CE vollstaendig",      "CE0005",            ERW_GLEICH);
    pruefe (F_CMD, "CF vollstaendig",      "CF0003",            ERW_GLEICH);
    pruefe (F_CMD, "MD vollstaendig",      "MD0005",            ERW_GLEICH);
    pruefe (F_CMD, "MF vollstaendig",      "MF0003",            ERW_GLEICH);
    pruefe (F_CMD, "AN ohne Name",         "AN00",              ERW_GLEICH);
    pruefe (F_CMD, "AN mit Name",          "AN00Fade",          ERW_GLEICH);
    pruefe (F_CMD, "OT vollstaendig",      "OT0002",            ERW_GLEICH);
    pruefe (F_CMD, "OI vollstaendig",      "OI0002",            ERW_GLEICH);
    pruefe (F_CMD, "OD vollstaendig",      "OD0002",            ERW_GLEICH);
    pruefe (F_CMD, "OC vollstaendig",      "OC0002",            ERW_GLEICH);
    pruefe (F_CMD, "OY vollstaendig",      "OY007f",            ERW_GLEICH);
    pruefe (F_CMD, "OF vollstaendig",      "OF0001",            ERW_GLEICH);
    pruefe (F_CMD, "OS vollstaendig",      "OS001231",          ERW_GLEICH);
    pruefe (F_CMD, "ON mit Text",          "ON00Geburtstag",    ERW_GLEICH);
    pruefe (F_CMD, "ON ohne Text",         "ON00",              ERW_GLEICH);
    pruefe (F_CMD, "I vollstaendig",       "I0001020304ab",     ERW_GLEICH);
    pruefe (F_CMD, "G Tetris",             "GTs",               ERW_GLEICH);
    pruefe (F_CMD, "G Snake",              "GSs",               ERW_GLEICH);

    printf ("\n4. Unbekannter Kommandobuchstabe -- MUSS durchgelassen werden (L269, Runde S)\n");
    pruefe (F_CMD, "m -- ESP sendet es, STM kennt es nicht", "m00010203", ERW_GLEICH);
    pruefe (F_CMD, "V Eroeffnungszeile Runde S", "V01",         ERW_GLEICH);
    pruefe (F_CMD, "Z Abschlussmarke Runde S",   "Z0102",       ERW_GLEICH);
    pruefe (F_CMD, "X nur Buchstabe",       "X",                ERW_GLEICH);
    pruefe (F_CMD, "unbekannt mit langem Rumpf", "QdiesIstEineZuordnungszeile", ERW_GLEICH);

    printf ("\n5. Die beiden exponierten Stellen (tftled.c, esp-spiffs.c)\n");
    pruefe (F_TFTLED, "DISP leer",           "",                ERW_GEFAHR);
    pruefe (F_TFTLED, "DISP nur eine Ziffer","0",               ERW_GEFAHR);
    pruefe (F_TFTLED, "DISP nur Zeilennummer","00",             ERW_GLEICH);
    pruefe (F_TFTLED, "DISP gueltig",        "00ESKISTAFUENF",  ERW_GLEICH);
    pruefe (F_ICON,   "ICON ungerade Laenge","0102030",         ERW_GEFAHR_BEGRENZT);
    pruefe (F_ICON,   "ICON ein Zeichen",    "0",               ERW_GEFAHR_BEGRENZT);
    pruefe (F_ICON,   "ICON leer",           "",                ERW_GLEICH);
    pruefe (F_ICON,   "ICON gueltig",        "01020304",        ERW_GLEICH);

    printf ("\n%d Faelle, %d Fehler, %lu Zeilen von der neuen Fassung verworfen\n",
            faelle, fehler, verworfen_gesamt);

    if (fehler)
    {
        printf ("PRUEFSTAND FEHLGESCHLAGEN\n");
        return 1;
    }
    printf ("Pruefstand bestanden\n");
    return 0;
}
