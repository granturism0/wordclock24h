/* Pruefstand A53 (Paket 2026-10-11, S3.6): Spiele zaehlen verworfene ESP-Zeilen (Ent-4 = V2).
 * WOERTLICH: die innere Pollschleife von tetris(), get_next_move() von snake(), game_count_dropped() und
 * schedule_esp8266_games() aus main.c. Attrappen: eine Zeilenquelle fuer esp8266_get_message(), log_printf.
 * Massstab ist eine EIGENE Einteilung der Rueckgabecodes (unten), nicht die Maske aus main.c. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#define ESP8266_TIMEOUT 0
#define ESP8266_OK 1
#define ESP8266_ERROR 2
#define ESP8266_DEBUGMSG 3
#define ESP8266_IPADDRESS 4
#define ESP8266_TIME 7
#define ESP8266_CMD 9
#define ESP8266_WEATHER 10
#define ESP8266_TABINFO 16
#define ESP8266_STATUS 25
#define ESP8266_NAK 26
#define ESP8266_SYNCVARS 27
#define ESP8266_ACK 28
#define ESP8266_ACK_DROP 29
#define ESP8266_UNSPECIFIED 0xFF
#define DISPLAY_CLOCK_FLAG_UPDATE_ALL 0x07
static struct { union { char cmd[64]; } u; uint_fast8_t is_online; } esp8266;
static uint_fast8_t var_sync_pending, display_clock_flag;
static uint32_t loops = 50, delay_loops = 50;
static void delay_msec (uint32_t m) { (void) m; }
static char logbuf[1024]; static int logzeilen;
static void log_printf (const char * f, ...) { va_list ap; va_start (ap, f); vsnprintf (logbuf, sizeof logbuf, f, ap); va_end (ap); logzeilen++; }
typedef enum { UP, DOWN, LEFT, RIGHT, QUIT } DIRECTION;

/* Zeilenquelle */
typedef struct { int code; const char * cmd; } ZEILE;
static ZEILE skript[400]; static int s_n, s_pos;
static int esp8266_get_message (void)
{
    if (s_pos >= s_n) return ESP8266_TIMEOUT;
    if (skript[s_pos].code == ESP8266_CMD) strcpy (esp8266.u.cmd, skript[s_pos].cmd);
    return skript[s_pos++].code;
}
void game_count_dropped (uint_fast8_t);
static void tetris (void);
static void snake (void);
#include "auszug.c"
static void tetris (void) { int ch, r = 0; do { ch = tetris_poll (); } while (ch != 'q' && ++r < 1000); }
static void snake (void)  { DIRECTION d = RIGHT; int r = 0; do { d = get_next_move (d); } while (d != QUIT && ++r < 1000); }

/* EIGENE Einteilung (Massstab): traegt die Zeile Zustand, den das Spiel verwirft? */
static int zaehlt (int code, const char * cmd, char spiel)
{
    if (code == ESP8266_TIMEOUT || code == ESP8266_OK || code == ESP8266_NAK || code == ESP8266_ACK || code == ESP8266_ACK_DROP || code == ESP8266_DEBUGMSG) return 0;
    if (code == ESP8266_CMD && cmd[0] == 'G' && cmd[1] == spiel) return 0;        /* vom Spiel ausgewertet */
    return 1;
}
static unsigned long faelle, fehl; static char was[300];
static void pruef (int ok, const char * w) { faelle++; if (! ok) { fehl++; if (fehl <= 8) printf ("  FEHL  %s\n", w); } }

static const ZEILE pool[] = {
    { ESP8266_CMD, "N0a05" }, { ESP8266_CMD, "N172e00" }, { ESP8266_CMD, "DC0001020300" }, { ESP8266_CMD, "GTl" }, { ESP8266_CMD, "GSr" },
    { ESP8266_CMD, "GTr" }, { ESP8266_CMD, "GSu" }, { ESP8266_CMD, "Gx" }, { ESP8266_IPADDRESS, "" }, { ESP8266_OK, "" }, { ESP8266_TIMEOUT, "" },
    { ESP8266_SYNCVARS, "" }, { ESP8266_NAK, "" }, { ESP8266_ACK, "" }, { ESP8266_ACK_DROP, "" }, { ESP8266_DEBUGMSG, "" }, { ESP8266_TIME, "" },
    { ESP8266_WEATHER, "" }, { ESP8266_TABINFO, "" }, { ESP8266_STATUS, "" }, { ESP8266_ERROR, "" }, { ESP8266_UNSPECIFIED, "" } };
#define NPOOL ((int) (sizeof pool / sizeof pool[0]))

static void lauf (char spiel, int n_zufall, unsigned seed, const char * name)
{
    int i, soll = 0;
    srand (seed); s_n = s_pos = 0;
    for (i = 0; i < n_zufall; i++)
    {
        ZEILE z = pool[rand () % NPOOL];
        if (z.code == ESP8266_CMD && z.cmd[0] == 'G' && z.cmd[1] == spiel && z.cmd[2] == 'q') continue;
        skript[s_n++] = z;
        soll += zaehlt (z.code, z.cmd, spiel);
    }
    skript[s_n].code = ESP8266_CMD; skript[s_n++].cmd = spiel == 'T' ? "GTq" : "GSq";     /* Spielende */
    var_sync_pending = 0; logzeilen = 0; logbuf[0] = 0; esp8266.is_online = 1;
    schedule_esp8266_games (spiel == 'T' ? "Ts" : "Ss");
    {
        char erwartet[64]; snprintf (erwartet, sizeof erwartet, "spiel: %u Zeilen verworfen\r\n", (unsigned) soll);
        snprintf (was, sizeof was, "%s (%c, %d Zeilen): Soll %d verworfen; var_sync_pending %d, Logzeilen %d \"%.40s\"",
                  name, spiel, s_n, soll, var_sync_pending, logzeilen, logbuf);
        if (soll > 0) pruef (var_sync_pending == 1 && logzeilen == 1 && ! strcmp (logbuf, erwartet), was);
        else          pruef (var_sync_pending == 0 && logzeilen == 0, was);
        pruef (s_pos == s_n, "Skript nicht vollstaendig gelesen (Spiel zu frueh beendet)");
    }
}

int main (void)
{
    int k; char name[32];
    /* benannte Faelle */
    s_n = 0;
    lauf ('T', 0, 1, "nur GTq");
    {   /* fest: GTl, CMC N.., IPADDRESS, ".", Timeout, GTr, Setter, GTq -> 2 fremde Zustandszeilen + Setter = 3 */
        static const ZEILE fest[] = { { ESP8266_CMD, "GTl" }, { ESP8266_CMD, "N172e00" }, { ESP8266_IPADDRESS, "" }, { ESP8266_OK, "" },
                                      { ESP8266_TIMEOUT, "" }, { ESP8266_CMD, "GTr" }, { ESP8266_CMD, "N0a05" }, { ESP8266_CMD, "GTq" } };
        memcpy (skript, fest, sizeof fest); s_n = 8; s_pos = 0; var_sync_pending = 0; logzeilen = 0; esp8266.is_online = 1;
        schedule_esp8266_games ("Ts");
        pruef (var_sync_pending == 1 && logzeilen == 1 && ! strcmp (logbuf, "spiel: 3 Zeilen verworfen\r\n"), "fest Tetris: nicht 'spiel: 3 Zeilen verworfen'");
        /* naechstes Spiel ohne fremde Zeile: keine Zeile -- der Zaehler wurde zurueckgesetzt */
        skript[0].code = ESP8266_CMD; skript[0].cmd = "GTq"; s_n = 1; s_pos = 0; var_sync_pending = 0; logzeilen = 0;
        schedule_esp8266_games ("Ts");
        pruef (var_sync_pending == 0 && logzeilen == 0, "zweites Spiel ohne fremde Zeile: Zaehler nicht zurueckgesetzt");
        /* Review S3.13 M1: ESP offline (CAP/OTA start im Spiel) -> Zeile JA, Vormerken NEIN, Zaehler zurueck */
        memcpy (skript, fest, sizeof fest); s_n = 8; s_pos = 0; var_sync_pending = 0; logzeilen = 0; esp8266.is_online = 0;
        schedule_esp8266_games ("Ts");
        pruef (var_sync_pending == 0 && logzeilen == 1 && ! strcmp (logbuf, "spiel: 3 Zeilen verworfen\r\n"), "ESP offline: vorgemerkt oder Zeile fehlt");
        esp8266.is_online = 1; skript[0].code = ESP8266_CMD; skript[0].cmd = "GTq"; s_n = 1; s_pos = 0; logzeilen = 0; var_sync_pending = 0;
        schedule_esp8266_games ("Ts");
        pruef (var_sync_pending == 0 && logzeilen == 0, "ESP offline: Zaehler nicht zurueckgesetzt");
        /* kein Spiel (anderer Parameter): nichts */
        s_n = 0; s_pos = 0; logzeilen = 0; schedule_esp8266_games ("Tx");
        pruef (logzeilen == 0, "GTx ohne Spiel: Logzeile");
    }
    for (k = 0; k < 300; k++)
    {
        snprintf (name, sizeof name, "zufall %d", k);
        lauf (k % 2 ? 'S' : 'T', 1 + k % 120, 1000 + k, name);
    }
    printf ("a53: %lu Pruefungen, %lu FEHL -- %s\n", faelle, fehl, fehl ? "FEHL" : "OK");
    return fehl ? 1 : 0;
}
