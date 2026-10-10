/* Pruefstand D (Teil D, STM-Seite; specs/paket-2026-10-09, design.md 5.8, AKD.5-AKD.8).
 *
 * Laesst den ECHTEN Firmware-Code laufen: esp8266_get_message() samt Praefixkette aus esp8266.c,
 * var_crc() und die Eroeffnungszeile aus vars.c, den Uebergabeblock esp8266_cmc_sync ->
 * var_sync_pending aus main.c -- ausgeschnitten von extract.py. Nachgebildet sind nur die
 * UART (Zeichen fuer Zeichen), die Uhr (uptime) und das Log.
 *
 * SCHIEDSRICHTER der Marken ist tools/checks/var-crc.c, hier unter dem Namen ref_crc eingebunden
 * -- nicht die Firmware-Rechnung. Die Tabelle der Schiedsrichter-Vektoren wird zusaetzlich gegen
 * dessen eigene Ausgabe verglichen (run.sh erzeugt x-vektoren.h aus dem Programmlauf).
 *
 * Modus ALT (-DALT): Erwartung des HEUTIGEN STM (AKD.8) -- jede CMC-Zeile ergibt
 * ESP8266_UNSPECIFIED und wird nicht angewandt. Soll gegen den Ausgangsstand BESTEHEN.
 *
 * Zieltypen (L256): mit -DPRUEFSTAND_SCHNELLE_TYPEN sind uint_fast8_t/uint_fast16_t 32 Bit wie auf
 * arm-none-eabi; run.sh uebersetzt und laeuft beide Fassungen.
 */
#include "pruefstand.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "esp8266.h"
#include "vars.h"

/* ---------- Schiedsrichter ---------- */
#define var_crc ref_crc
#define main    ref_main
#include "var-crc.c"
#undef var_crc
#undef main
#include "x-vektoren.h"                                 /* REF_VEKTOREN: { "nutzlast", "*hhhh" } aus dem Lauf von var-crc.c */

/* ---------- Umgebung des Ziels ---------- */
volatile uint32_t   uptime;
ESP8266_GLOBALS     esp8266;
static uint_fast8_t var_sync_pending;                   /* main.c: static, hier fuer den Uebergabeblock */
static int          verbose;

static void log_flush (void) {}
static void log_puts (const char * s) { if (verbose) fputs (s, stdout); }
static char logbuf[1 << 16];
static size_t logpos;
static int  log_printf (const char * fmt, ...)
{
    va_list ap; int n;
    if (logpos > sizeof (logbuf) - 512) logpos = 0;      /* nur die juengsten Zeilen zaehlen, siehe log_take() */
    va_start (ap, fmt); n = vsnprintf (logbuf + logpos, sizeof (logbuf) - logpos, fmt, ap); va_end (ap);
    if (verbose) fputs (logbuf + logpos, stdout);
    logpos += (size_t) n;
    return 0;
}
static void log_take (char * dst, size_t len) { snprintf (dst, len, "%.*s", (int) logpos, logbuf); logpos = 0; logbuf[0] = 0; }
#define debug_log_printf(...)
#define debug_log_flush()
#define debug_log_message(s)

/* UART: genau eine Zeile steht an, Zeichen fuer Zeichen */
static char rx[600];
static size_t rx_pos, rx_len;
uint_fast8_t esp8266_uart_char_available (void) { return rx_pos < rx_len; }
uint_fast8_t esp8266_uart_poll (uint_fast8_t * ch) { if (rx_pos >= rx_len) return 0; *ch = (uint_fast8_t) (unsigned char) rx[rx_pos++]; return 1; }
void esp8266_uart_puts (const char * s) { (void) s; }
void esp8266_uart_flush (void) {}

#include SNIPPET

/* ---------- Auswertung ---------- */
static int faelle, fehl;
static void ok (int cond, const char * was, const char * zeile)
{
    faelle++;
    if (! cond) { fehl++; printf ("FEHL  %s: [%s]\n", was, zeile); }
}

#define SENTINEL "--unberuehrt--"
static unsigned long erwartet_rejects;                  /* eigene Zaehlung des Pruefstands, saettigend wie gefordert */
static unsigned long vormerkungen;

/* Eine Zeile zustellen; liefert rtc. Laeuft danach den Hauptloop-Block aus main.c. */
static uint_fast8_t zustellen (const char * zeile)
{
    uint_fast8_t rtc;
    strcpy (esp8266.u.cmd, SENTINEL);
    snprintf (rx, sizeof (rx), "%s\r\n", zeile); rx_len = strlen (rx); rx_pos = 0;
    rtc = esp8266_get_message ();
#if MAIN_HOOK
    var_sync_pending = 0;
    main_hook ();
    if (var_sync_pending) vormerkungen++;
#endif
    return rtc;
}

static void cmc (char * out, size_t len, const char * payload)  /* so wie der ESP sie bildet: "*%04x", klein */
{
    snprintf (out, len, "CMC %s*%04x", payload, (unsigned int) ref_crc (payload));
}

/* Erwartung: angewandt wie "CMD <payload>" */
static void erwarte_angewandt (const char * zeile, const char * payload, const char * was)
{
    char log[512], ref_cmd[300], ref_u[ESP8266_MAX_CMD_LEN + 1];
    uint_fast8_t rtc, rtc_cmd;
    unsigned long vm = vormerkungen;

    snprintf (ref_cmd, sizeof (ref_cmd), "CMD %s", payload);
    rtc_cmd = zustellen (ref_cmd); strcpy (ref_u, esp8266.u.cmd); log_take (log, sizeof (log));
    rtc = zustellen (zeile); log_take (log, sizeof (log));
#ifdef ALT
    ok (rtc == ESP8266_UNSPECIFIED, "AKD.8 alter STM: CMC ergibt UNSPECIFIED", zeile);
    ok (! strcmp (esp8266.u.cmd, SENTINEL), "AKD.8 alter STM: nichts angewandt", zeile);
    (void) rtc_cmd; (void) was; (void) vm;
#else
    ok (rtc == ESP8266_CMD && rtc_cmd == ESP8266_CMD, was, zeile);
    ok (! strcmp (esp8266.u.cmd, ref_u) && ! strcmp (esp8266.u.cmd, payload), "AKD.6 Nutzlast gleich CMD", zeile);
    ok (strstr (log, "abgewiesen") == 0, "AKD.6 keine Abweisungszeile", zeile);
    ok (vormerkungen == vm, "AKD.6 kein Vormerken", zeile);
#endif
}

/* Erwartung: abgewiesen, gezaehlt, gedrosselt gemeldet */
static void erwarte_abgewiesen (const char * zeile, const char * was)
{
    char log[512], soll[80];
    uint_fast8_t rtc = zustellen (zeile);
    unsigned int len = (unsigned int) strlen (zeile + 4);

    log_take (log, sizeof (log));
    ok (rtc != ESP8266_CMD, was, zeile);
    ok (! strcmp (esp8266.u.cmd, SENTINEL), "nicht angewandt (u.cmd unberuehrt)", zeile);
#ifdef ALT
    ok (rtc == ESP8266_UNSPECIFIED, "AKD.8 alter STM: UNSPECIFIED", zeile);
    (void) soll; (void) len;
#else
    if (erwartet_rejects < 0xFFFF) erwartet_rejects++;
    if (erwartet_rejects <= 4 || erwartet_rejects % 50 == 0)
    {
        snprintf (soll, sizeof (soll), "cmd abgewiesen #%lu len=%u\r\n", erwartet_rejects, len);
        ok (! strcmp (log, soll), "AKD.7 Zeile erwartet (erste vier, dann jede fuenfzigste)", zeile);
        if (strcmp (log, soll) && fehl < 20) printf ("      soll [%s] ist [%s]\n", soll, log);
    }
    else
    {
        ok (log[0] == 0, "AKD.7 gedrosselt: keine Zeile", zeile);
    }
    ok (strstr (log, "*") == 0 && (log[0] == 0 || strncmp (log, "cmd abgewiesen #", 16) == 0), "AKD.7 kein Wert in der Zeile", zeile);
#endif
}

int main (int argc, char ** argv)
{
    char z[400], p[300], l1[200], l2[200];
    size_t i, j, k;
    unsigned long vm;
    verbose = argc > 1;

    uptime = 5;                                                     /* kurz nach dem Start */

    /* ---- Schiedsrichter gegen die eigene Ausgabe von var-crc.c, und die Firmware gegen beide ---- */
    for (i = 0; i < sizeof (REF_VEKTOREN) / sizeof (REF_VEKTOREN[0]); i++)
    {
        char m[8];
        snprintf (m, sizeof (m), "*%04x", (unsigned int) ref_crc (REF_VEKTOREN[i][0]));
        ok (! strcmp (m, REF_VEKTOREN[i][1]), "Schiedsrichter: Tabelle = Rechnung", REF_VEKTOREN[i][0]);
        ok (var_crc (REF_VEKTOREN[i][0]) == ref_crc (REF_VEKTOREN[i][0]), "Firmware-var_crc = Schiedsrichter", REF_VEKTOREN[i][0]);
    }

    /* ---- AKD.5: Eroeffnung ---- */
#ifndef ALT
    ok (VB_SPRINTF_COUNT == 1, "AKD.5 genau eine Eroeffnungszeile in vars.c", "VB");
#if VB_SPRINTF_COUNT == 1
    for (k = 0; k < 2; k++)
    {
        unsigned int f;
        esp8266.cap_var_crc = (uint_fast8_t) k;
        vb_frame (z, 0x2a);
        f = (unsigned int) strtoul (z + 2, 0, 0) ; f = 0; sscanf (z + 2, "%2x", &f);
        ok (strlen (z) == 6 && ! strncmp (z, "VB", 2) && ! strcmp (z + 4, "2a"), "AKD.5 Form VBffnn", z);
        ok ((f & 0x04) != 0, "AKD.5 Flag 0x04 gesetzt", z);
        ok ((f & 0x02) != 0 && (f & 0x01) == k, "AKD.5 0x02 und 0x01 wie bisher", z);
    }
    esp8266.cap_var_crc = 0;
#endif
#endif

    /* ---- AKD.6: richtige Marke => angewandt wie CMD ---- */
    for (i = 0; i < sizeof (REF_VEKTOREN) / sizeof (REF_VEKTOREN[0]); i++)
    {
        if (! REF_VEKTOREN[i][0][0]) continue;                      /* leere Nutzlast: unten bei den Abweisungen */
        snprintf (z, sizeof (z), "CMC %s%s", REF_VEKTOREN[i][0], REF_VEKTOREN[i][1]);
        erwarte_angewandt (z, REF_VEKTOREN[i][0], "AKD.6 CMC richtig => angewandt (Schiedsrichter-Vektor)");
    }
    {   /* laengste regulaere Nutzlast: 'S' + 2 + 63 */
        memset (p, 'x', sizeof (p)); memcpy (p, "S03", 3); p[66] = 0;
        cmc (z, sizeof (z), p);
        ok (strlen (z + 4) <= ESP8266_MAX_CMD_LEN + 5 && strlen (p) + 5 <= ESP8266_MAX_CMD_LEN, "Laenge: S+63 samt Marke passt in ESP8266_MAX_CMD_LEN", z);
        ok (strlen (z) + 2 <= ESP8266_MAX_ANSWER_LEN, "Laenge: Zeile passt in den Zeilenpuffer", z);
        erwarte_angewandt (z, p, "AKD.6 laengste Nutzlast S+63 angewandt");
    }
    {   /* Grenze u.cmd: 127 angewandt, 128 abgewiesen */
        memset (p, 'y', sizeof (p)); p[0] = 'S'; p[ESP8266_MAX_CMD_LEN] = 0;
        cmc (z, sizeof (z), p); erwarte_angewandt (z, p, "Grenze: Nutzlast 127 angewandt");
    }
    erwarte_angewandt ("CMC N0a0f*487a", "N0a0f", "AKD.6 Marke byte-gleich %04x");
    cmc (z, sizeof (z), "S03a*b*c"); erwarte_angewandt (z, "S03a*b*c", "AKD.6 '*' in der Nutzlast");
    cmc (z, sizeof (z), "S03ab*c328"); erwarte_angewandt (z, "S03ab*c328", "AKD.6 Nutzlast endet selbst auf *hhhh");

    /* ---- CMD wie heute ---- */
    {
        uint_fast8_t rtc;
        char log[512];
        rtc = zustellen ("CMD N0a0f"); log_take (log, sizeof (log));
        ok (rtc == ESP8266_CMD && ! strcmp (esp8266.u.cmd, "N0a0f"), "CMD ohne Marke angewandt", "CMD N0a0f");
        rtc = zustellen ("CMD S03meinhost*c328"); log_take (log, sizeof (log));
        ok (rtc == ESP8266_CMD && ! strcmp (esp8266.u.cmd, "S03meinhost*c328"), "CMD mit Text auf *hhhh: unveraendert angewandt", "CMD S03meinhost*c328");
        rtc = zustellen ("CMD S03meinhost*0000"); log_take (log, sizeof (log));
        ok (rtc == ESP8266_CMD && ! strcmp (esp8266.u.cmd, "S03meinhost*0000"), "CMD mit falscher Marke im Text: unveraendert angewandt", "CMD S03meinhost*0000");
        ok (strstr (log, "abgewiesen") == 0, "CMD: keine Abweisungszeile", "CMD");
    }

    /* ---- AKD.7: Fehlerformen ---- */
    erwarte_abgewiesen ("CMC N0a0f*487b", "falsche Marke (eine Ziffer)");
    erwarte_abgewiesen ("CMC N0a0f*a487", "falsche Marke (vertauscht)");
    erwarte_abgewiesen ("CMC OT0*846b", "verkuerzt: OT0002 auf OT0, Marke von OT0002 (L205)");
    erwarte_abgewiesen ("CMC S03meinhst*c328", "verkuerzt: ein Byte in der Mitte verloren");
    erwarte_abgewiesen ("CMC N0a0f", "ohne Marke");
    erwarte_abgewiesen ("CMC N0a0f*", "nur Stern");
    erwarte_abgewiesen ("CMC N0a0f*487", "drei Hexziffern");
    erwarte_abgewiesen ("CMC N0a0*487a", "drei Hexziffern nach Verlust (Laenge stimmt nicht)");
    erwarte_abgewiesen ("CMC N0a0f*487a0", "fuenf Hexziffern");
    erwarte_abgewiesen ("CMC N0a0f*48g7", "Nicht-Hex g");
    ok (ref_crc ("N0a38") == 0x204f, "Vektor fuer den htoi-Fall: Marke hat eine Null-Ziffer", "N0a38");
    erwarte_abgewiesen ("CMC N0a38*2g4f", "Nicht-Hex an einer Null-Stelle (htoi naehme es still als 0, L232)");
    erwarte_abgewiesen ("CMC N0a0f*48 a", "Nicht-Hex Leerzeichen");
    erwarte_abgewiesen ("CMC N0a0f*-87a", "Nicht-Hex Vorzeichen (htoi/strtol nimmt es)");
    erwarte_abgewiesen ("CMC N0a0f*0x7a", "Nicht-Hex 0x (strtol nimmt es)");
    erwarte_abgewiesen ("CMC N0a0f*487A", "Grossbuchstabe (der ESP sendet %04x)");
    erwarte_abgewiesen ("CMC N0a0f+487a", "Trenner kein Stern");
    erwarte_abgewiesen ("CMC *0000", "leere Nutzlast mit richtiger Marke");
    erwarte_abgewiesen ("CMC *", "leere Nutzlast ohne Marke");
    erwarte_abgewiesen ("CMC ", "ganz leer");
    {   memset (p, 'y', sizeof (p)); p[0] = 'S'; p[ESP8266_MAX_CMD_LEN + 1] = 0;
        cmc (z, sizeof (z), p); erwarte_abgewiesen (z, "Grenze: Nutzlast 128 mit richtiger Marke (nicht gekuerzt angewandt)"); }
#ifdef PRUEFSTAND_SCHNELLE_TYPEN
    /* Nur mit Zieltypen: answer_pos ist uint_fast8_t, auf dem Mac 8 Bit -- dort liefe der Zeilenpuffer-
     * Index bei 256 ueber (Warnung beim Uebersetzen), auf dem STM32 (32 Bit) nicht. Ein Hostartefakt
     * des bestehenden Codes, kein Befund (L256). */
    {   memset (p, 'y', sizeof (p)); p[0] = 'S'; p[290] = 0;
        cmc (z, sizeof (z), p); erwarte_abgewiesen (z, "ueber den Zeilenpuffer (256) hinaus, Marke abgeschnitten"); }
#endif

    /* ---- zwei CMC-Zeilen verschmolzen, jeder Schnitt ---- */
    {
        static const char * paare[][2] = {
            { "N0a0f", "S03meinhost" }, { "OT0002", "OT0003" }, { "N0a0f", "N0a10" },
            { "S03meinhost", "S04updates.example.org" }, { "DC00ff800000ff", "T0120261004213000" },
        };
        unsigned long n = 0;
        for (k = 0; k < sizeof (paare) / sizeof (paare[0]); k++)
        {
            int r;
            for (r = 0; r < 2; r++)
            {
                cmc (l1, sizeof (l1), paare[k][r]); cmc (l2, sizeof (l2), paare[k][1 - r]);
                for (i = 0; i <= strlen (l1); i++)
                    for (j = 0; j <= strlen (l2); j++)
                    {
                        snprintf (z, sizeof (z), "%.*s%s", (int) i, l1, l2 + j);
                        if (! strcmp (z, l1) || ! strcmp (z, l2) || strncmp (z, "CMC ", 4)) continue;  /* kein Verschmelzen bzw. keine CMC-Zeile mehr (unten) */
                        erwarte_abgewiesen (z, "verschmolzen"); n++;
                    }
                /* Schnitte, die das Praefix treffen: Was dabei herauskommt, darf nie angewandt werden */
                for (i = 0; i < 4; i++)
                    for (j = 0; j <= strlen (l2); j++)
                    {
                        uint_fast8_t rtc; char log[512];
                        snprintf (z, sizeof (z), "%.*s%s", (int) i, l1, l2 + j);
                        if (! strcmp (z, l2) || ! z[0]) continue;
                        rtc = zustellen (z); log_take (log, sizeof (log));
                        if (! strncmp (z, "CMC ", 4)) { if (erwartet_rejects < 0xFFFF) erwartet_rejects++; }   /* zaehlt mit, Zeile hier nicht geprueft */
                        ok (rtc != ESP8266_CMD, "verschmolzen im Praefix: nicht angewandt, keine beginnt mit CMD", z);
                        n++;
                    }
            }
        }
        printf ("  verschmolzene Zeilen geprueft: %lu\n", n);
    }

#ifndef ALT
    /* ---- N1: Vormerken hoechstens einer je 60 s ---- */
#if MAIN_HOOK
    uptime = 100000;
    vm = vormerkungen; erwarte_abgewiesen ("CMC N0a0f*0000", "N1 a"); erwarte_abgewiesen ("CMC N0a0f*0001", "N1 b");
    ok (vormerkungen - vm == 1, "N1 zwei Abweisungen in derselben Sekunde => ein Vormerken", "");
    vm = vormerkungen; uptime += 61; erwarte_abgewiesen ("CMC N0a0f*0000", "N1 c"); uptime += 10; erwarte_abgewiesen ("CMC N0a0f*0000", "N1 d");
    ok (vormerkungen - vm == 1, "N1 zwei Abweisungen in 10 s => ein Vormerken", "");
    vm = vormerkungen; uptime += 61; erwarte_abgewiesen ("CMC N0a0f*0000", "N1 e"); uptime += 61; erwarte_abgewiesen ("CMC N0a0f*0000", "N1 f");
    ok (vormerkungen - vm == 2, "N1 zwei Abweisungen im Abstand von 61 s => zwei", "");
    vm = vormerkungen; uptime += 61; erwarte_abgewiesen ("CMC N0a0f*0000", "N1 g"); uptime += 59; erwarte_abgewiesen ("CMC N0a0f*0000", "N1 h");
    ok (vormerkungen - vm == 1, "N1 Abstand 59 s => ein Vormerken", "");
    vm = vormerkungen; uptime += 61;
    for (i = 0; i < 100; i++) erwarte_abgewiesen ("CMC N0a0f*0000", "N1 Stoss");
    ok (vormerkungen - vm == 1, "N1 Stoss von 100 Abweisungen => ein Vormerken", "");
    vm = vormerkungen; uptime += 61; erwarte_angewandt ("CMC N0a0f*487a", "N0a0f", "N1 richtige Zeile");
    ok (vormerkungen == vm, "N1 richtige Zeile merkt nichts vor", "");
    ok (esp8266_cmc_sync == 0, "N1 Hauptloop loescht esp8266_cmc_sync", "");
#else
    ok (0, "N1: Uebergabeblock esp8266_cmc_sync -> var_sync_pending fehlt in main.c", "");
#endif
    /* ---- Zaehler saettigend ---- */
    {
        char log[512];
        while (erwartet_rejects < 0xFFFF + 20UL) { zustellen ("CMC N0a0f*0000"); log_take (log, sizeof (log)); erwartet_rejects++;
            if (erwartet_rejects > 0xFFFF) ok (log[0] == 0, "Zaehler saettigend: nach 65535 keine Zeile mehr (kein Umlauf auf #1)", log); }
        erwartet_rejects = 0xFFFF;
        erwarte_abgewiesen ("CMC N0a0f*0000", "nach der Saettigung weiter abgewiesen");
    }
    (void) vm;
#else
    (void) vm;
#endif

    printf ("%s: %d Faelle, %d fehlgeschlagen\n", fehl ? "FEHL" : "OK", faelle, fehl);
    return fehl ? 1 : 0;
}
