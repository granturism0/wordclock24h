/*-------------------------------------------------------------------------------------------------------------------------------------------
 * vars.c - synchronisation of variables/parameters between STM32 and ESP8266
 *
 * Copyright (c) 2016-2026 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#include <stdio.h>

#include "vars.h"
#include "display.h"
#include "overlay.h"
#include "eep.h"
#include "eeprom-data.h"
#include "main.h"
#include "power.h"
#include "ldr.h"
#include "tempsensor.h"
#include "ds18xx.h"
#include "rtc.h"
#include "timeserver.h"
#include "night.h"
#include "alarm.h"
#include "weather.h"
#include "dfplayer.h"
#include "ssd1963.h"
#include "remote-ir.h"
#include "delay.h"

#include "log.h"
#include "esp8266.h"

#undef UART_PREFIX
#define UART_PREFIX         esp8266
#include "uart.h"

typedef struct tm   TM;
uint_fast8_t        var_send_busy;

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send buffer to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
#define VAR_SEND_TIMEOUT_SEC    3                               // deutlich unter den 20s des Watchdogs

static uint_fast8_t var_send_nested = 0;                        // eigener Wiedereintrittsschutz

/* Zeitbudget fuer den Watchdog-Reload nach eingetroffener Quittung, je Hauptloop-Durchlauf.
 *
 * Die Quittungsbedingung allein deckt nur zwei Lagen ab -- Bruecke antwortet, Bruecke tot.
 * Die dritte fehlte: Antwortet die Bruecke SPORADISCH, bedient jede eintreffende Quittung den
 * Watchdog erneut, und var_send_all_variables() darf mit rund 194 Kommandos a bis zu
 * VAR_SEND_TIMEOUT_SEC bis zu zehn Minuten laufen. Die Uhr steht waehrenddessen.
 *
 *   Lage der Bruecke                             ohne Budget              mit Budget
 *   ----------------------------------------------------------------------------------------
 *   antwortet zuegig (Millisekunden)             kein Reload noetig       unveraendert
 *   antwortet langsam (bis ~150ms je Kommando)   laeuft durch             laeuft durch, das
 *                                                                         Budget traegt einen
 *                                                                         Burst von 194
 *   antwortet sporadisch (Sekunden je Kommando)  bis zu 600s Stillstand   hoechstens 30s, dann
 *                                                                         greift der Watchdog
 *   antwortet nicht                              Reset nach rund sieben   unveraendert
 *                                                Kommandos
 *
 * Schlechtester Fall danach: 30s Budget + bis zu 20s Watchdog-Fenster = 50s Stillstand. Gegen
 * den Stand vor 9eb6dd9 (20s) eine bewusste Verschlechterung, gegen den Stand danach (600s)
 * eine Verbesserung um den Faktor zwanzig -- der Preis dafuer, dass ein langsamer, aber
 * lebendiger ESP seinen Variablensatz vollstaendig bekommt.
 *
 * Zeit und nicht Zahl der Reloads, weil die Zahl nichts ueber den Stillstand sagt: 194 schnelle
 * Kommandos sind harmlos, sieben langsame nicht. Geschuetzt wird die Uhr, nicht die Bruecke.
 * Wer den Wert aendert, sieht an der Tabelle, wogegen er tauscht.
 */
#define VAR_SEND_RELOAD_BUDGET_SEC  30

/* Nullpunkt des Budgets. Zurueckgesetzt wird er am Kopf des Hauptloops, an derselben Stelle wie
 * der regulaere watchdog_reload() (main.c), ueber var_send_reload_budget_reset(). GESETZT wird
 * er erst beim ersten wartenden Aufruf danach -- nicht im Loopkopf selbst, sonst zaehlte das
 * Budget die Zeit fuer Anzeige, RTC und Temperatur mit und waere aufgebraucht, bevor das erste
 * Kommando ueberhaupt draussen ist.
 *
 * Keine neue watchdog_reload()-Aufrufstelle. Den gueltigen Bestand nennt Guardrail S7, nicht
 * dieser Kommentar: Hier stand bis zum 04.10.2026 eine feste Zahl, und die war beim Lesen
 * bereits falsch -- display.c:4254 (display_wait_for_tables) ist mit A22 dazugekommen. Eine
 * Zahl, die mit dem Code waechst, veraltet still; dieselbe Gattung wie die Versionsnummern in
 * der Doku, vor denen CLAUDE.md an genau dieser Stelle warnt.
 */
static uint32_t     var_send_reload_start = 0;                  // uptime beim ersten wartenden Aufruf dieses Durchlaufs
static uint_fast8_t var_send_reload_armed = 0;                  // Nullpunkt in diesem Durchlauf bereits gesetzt?

void
var_send_reload_budget_reset (void)
{
    var_send_reload_armed = 0;
}

/* Zwei Messfelder fuer die Diagnosezeile (specs/bruecke, Design 3). Sie beantworten EINE Frage,
 * die bis heute offen ist: Welcher der beiden Verlustwege aus BEFUNDE.md L107 laeuft, wenn der
 * ESP neu startet und der Variablensatz danach beschaedigt ist?
 *
 *   <timeouts>       Weg (A): ein Kommando ist in den 3-Sekunden-Abbruch gelaufen und endgueltig
 *                    weg. Erhoeht im Zeitzweig der Warteschleife.
 *   <verschachtelt>  Weg (B): ein Aufruf kehrte bei var_send_nested sofort zurueck, also OHNE
 *                    jede Quittungspruefung. Laeuft ein ganzer Vollabgleich so, gehen rund 194
 *                    Kommandos mit Leitungsgeschwindigkeit in den 256-Byte-Empfangsring des ESP.
 *
 * Deutung nach EINEM ESP-Neustart:
 *
 *   timeouts   verschachtelt   Abzug beschaedigt   Folgerung
 *   ----------------------------------------------------------------------------------------
 *   klein      springt um ~190 ja                  Weg (B), der Massenverlust
 *   steigt     klein           ja                  Weg (A), einzelne Kommandos im Timeout
 *   steigt     springt         ja                  beide, nacheinander
 *   klein      klein           ja                  KEINER von beiden -- die Ursache liegt dann
 *                                                  nicht in var_send_buf()
 *   klein      klein           nein                der Abgleich lief sauber durch; ein einzelner
 *                                                  Durchgang ohne Vorfall belegt nichts
 *
 * Saettigend statt umlaufend: Eine umlaufende Zahl liest sich als kleine Zahl und luegt dabei.
 * Beide sind seit dem STM-Start kumulativ; nach einem Reset stehen sie auf 0, und dass ein Reset
 * war, zeigt die Folgenummer seq, die wieder bei 1 beginnt.
 *
 * Im Ruhebetrieb muessen beide bei 0 bleiben. Steigt <timeouts> dort, kommt die Punkt-Quittung
 * nicht an -- dann ist Design 1 falsch umgesetzt und nicht "fast richtig" (AK2).
 */
static uint16_t     var_send_timeout_cnt = 0;                   // Weg (A): Abbrueche nach VAR_SEND_TIMEOUT_SEC
static uint16_t     var_send_nested_cnt = 0;                    // Weg (B): quittungsfreie Eintritte ueber var_send_nested

uint_fast16_t
var_send_timeout_count (void)
{
    return var_send_timeout_cnt;
}

uint_fast16_t
var_send_nested_count (void)
{
    return var_send_nested_cnt;
}

/* Pruefsumme der var-Zeile (A32, Baustein 2, Design 6.1).
 *
 * Fletcher-artig, ohne Tabelle und ohne Division. Die LAENGE ist der Startwert -- damit wirkt sich
 * eine Laengenaenderung aus, und genau die ist der belegte Schadensfall: Der Empfangsring verliert
 * zusammenhaengende Bloecke von rund 77 Byte (L141, L188), aus zwei Zeilen wird eine kuerzere, und
 * der ESP liest sie als gueltig (L205).
 *
 * Gerechnet wird ueber die Nutzlast OHNE "var " und OHNE die Pruefsumme selbst. Dieselbe Rechnung
 * steht ein zweites Mal im ESP (C++) -- das sind zwei Wahrheiten. Schiedsrichter ist eine dritte,
 * unabhaengig und VOR beiden entstandene Umsetzung mit festen Vektoren: tools/checks/var-crc.c.
 *
 * 16 Bit und nicht 8: Zwei zusaetzliche Byte je Kommando sind beim Vollabgleich rund 390 Byte auf
 * 3 kB, also nicht messbar; das Restrisiko einer unerkannt verfaelschten Zeile faellt dabei von
 * rund 0,4 % auf rund 0,0015 %. Garantien gibt die Rechnung keine -- sie ist eine Pruefsumme und
 * keine Sicherung. Erkannt heisst hier: nachgesendet.
 */
static uint16_t
var_crc (const char * payload)
{
    uint8_t         sum1;
    uint8_t         sum2 = 0;
    uint_fast16_t   len  = (uint_fast16_t) strlen (payload);
    uint_fast16_t   i;

    sum1 = (uint8_t) len;

    for (i = 0; i < len; i++)
    {
        sum1 = (uint8_t) (sum1 + (uint8_t) payload[i]);
        sum2 = (uint8_t) (sum2 + sum1);
    }

    return (uint16_t) ((sum2 << 8) | sum1);
}

/* Nachsendeliste (A32, Baustein 1, Design 3.2).
 *
 * Bis hierher galt: Bleibt die Quittung aus, steht im Code ein break, und das Kommando ist weg
 * (L230). Nachgesendet wird NICHT in der Warteschleife -- dreimal drei Sekunden waeren neun
 * Sekunden Hauptloop-Blockade und damit ein Rueckfall hinter A29/A31 (L204, L211, L226). Beim
 * Fehlschlag wird deshalb nur VORGEMERKT; gesendet wird spaeter im Hauptloop, hoechstens ein
 * Versuch je Sekunde, nach dem Muster des IR-Abzugs (main.c).
 *
 * Jede Konstante nennt, wogegen sie tauscht -- so wie VAR_SEND_RELOAD_BUDGET_SEC es vormacht.
 */

/* Wie viele Kommandos gleichzeitig vorgemerkt sein duerfen. Das ist der eigentliche Deckel bei
 * toter Bruecke: vier Plaetze mal drei Versuche sind hoechstens zwoelf Nachsendungen je Stoerung.
 * Tauscht RAM gegen Deckung -- 4 x 80 Byte .bss. Reicht das auf dem F103 (20 kB) nicht, faellt der
 * Wert auf 2; genau dafuer ist es eine Konstante (Risiko R-4). Nach oben lohnt es nicht: Laeuft die
 * Liste voll, ist die Einzelreparatur ueberfordert, und richtig waere dann der Vollabgleich.
 */
#define VAR_RETRY_SLOTS             4

/* Laengstes vorzumerkendes Kommando. Haengt an der laengsten Zeichenkettenvariablen: "S" + zwei
 * Indexziffern + Wert, beim Hostnamen also 3 + 64 = 67. Waechst eine solche Variable ueber diese
 * Grenze, wird NICHT still verworfen, sondern var_retry_toolong_cnt gezaehlt -- tauscht RAM gegen
 * Sichtbarkeit. Ein stilles Verwerfen waere genau der Fehler, gegen den dieser ganze Abschnitt
 * antritt.
 */
#define VAR_RETRY_CMD_LEN          72

/* Zusaetzliche Versuche je Kommando. Tauscht Zustellwahrscheinlichkeit gegen Last auf genau der
 * Leitung, deren Ueberlastung die Quittung gekostet hat (Mitkopplung, L109). Zwei Versuche decken
 * rund zehn Sekunden Bruecken-Stoerung je Kommando ab; ein ESP-Neustart liegt darunter, ein
 * OTA-Schreibvorgang darueber -- dagegen hilft nur der Vollabgleich, nicht eine groessere Zahl.
 */
#define VAR_RETRY_MAX_ATTEMPTS      2

/* Fruehestens so viele Sekunden nach dem Fehlschlag. Tauscht Reaktionszeit gegen die Chance, dass
 * sich die Gegenstelle erholt: sofort nachzusenden traefe dieselbe Ueberlast ein zweites Mal.
 * Beim ausdruecklich abgelehnten Kommando (!v) gilt das nicht -- dort lebt die Bruecke, und es
 * wird mit due = uptime sofort faellig.
 */
#define VAR_RETRY_DELAY_SEC         2

/* Hoechstens ein Versuch je Sekunde, ueber ALLE Eintraege. Tauscht Durchsatz gegen die Zusicherung,
 * dass nie zwei Nachsendungen in demselben Hauptloop-Durchlauf liegen: Der laengste
 * zusammenhaengende Stillstand bleibt damit bei VAR_SEND_TIMEOUT_SEC und waechst nicht (AK1).
 */
#define VAR_RETRY_SPACING_SEC       1

/* Hoechstens eine "Liste voll"-Zeile je 60 s. Tauscht Vollstaendigkeit der Meldung gegen dieselbe
 * Mitkopplung wie oben: Jede Logzeile legt rund 60 Byte auf die ueberlastete Leitung. Wie viele
 * Eintraege verworfen wurden, sagt der Zaehler -- nicht die Zahl der Zeilen.
 */
#define VAR_RETRY_FULL_LOG_SEC     60

/* Laengste Kennung: "n" + 2 Indexziffern + 2 Feldziffern = 5 (var_send_num8_array). */
#define VAR_RETRY_KEY_LEN           5

/* Feldreihenfolge ist hier nicht Geschmack: Steht due hinter cmd[73], schiebt die Ausrichtung
 * drei Fuellbyte dazwischen und der Platz waechst von 80 auf 84 Byte. Vorangestellt passt alles
 * ohne Luecke -- 16 Byte weniger .bss ueber vier Plaetze, umsonst zu haben.
 */
typedef struct
{
    uint32_t        due;                                        // uptime, ab der gesendet werden darf
    char            cmd[VAR_RETRY_CMD_LEN + 1];                 // Nutzlast ohne "var " und ohne Pruefsumme; cmd[0] == '\0' heisst "Platz frei"
    uint8_t         idlen;                                      // Laenge der Kennung, siehe var_retry_purge()
    uint8_t         attempts;                                   // bereits unternommene Nachsendeversuche
} VAR_RETRY_SLOT;

static VAR_RETRY_SLOT   var_retry_slots[VAR_RETRY_SLOTS];
static uint32_t         var_retry_last_attempt = 0;             // uptime der letzten Nachsendung, Taktung ueber VAR_RETRY_SPACING_SEC
static uint32_t         var_retry_full_log_last = 0;            // uptime der letzten "Liste voll"-Zeile
static uint_fast8_t     var_retry_full_logged = 0;              // erste Meldung nicht durch die Drossel verschlucken
static uint_fast8_t     var_retry_attempts_in = 0;              // vom Drain gesetzt: Zahl der Versuche DIESES Aufrufs von var_send_buf()

/* Vier saettigende Zaehler, dieselbe Bauart wie var_send_timeout_cnt: 65535 heisst "mindestens
 * 65535". Sie werden NICHT in die Diagnosezeile aufgenommen -- die steht bei 118 von 119 Zeichen
 * (main.c), es passt kein Feld mehr hinein. Sichtbar werden sie ereignisgetrieben ueber die drei
 * Logzeilen unten; die Zugriffsfunktionen gibt es fuer den Tag, an dem jemand Platz schafft.
 */
static uint16_t     var_retry_ok_cnt = 0;                       // Nachsendungen, die quittiert wurden
static uint16_t     var_retry_gaveup_cnt = 0;                   // nach VAR_RETRY_MAX_ATTEMPTS aufgegeben
static uint16_t     var_retry_dropped_cnt = 0;                  // Liste war voll
static uint16_t     var_retry_toolong_cnt = 0;                  // Kommando laenger als VAR_RETRY_CMD_LEN

uint_fast16_t
var_retry_ok_count (void)
{
    return var_retry_ok_cnt;
}

uint_fast16_t
var_retry_gaveup_count (void)
{
    return var_retry_gaveup_cnt;
}

uint_fast16_t
var_retry_dropped_count (void)
{
    return var_retry_dropped_cnt;
}

uint_fast16_t
var_retry_toolong_count (void)
{
    return var_retry_toolong_cnt;
}

/* Die Kennung fuer eine Logzeile, nullterminiert. In die Logzeilen gehoert die KENNUNG und niemals
 * der Wert: Die Zeichenkettenvariablen tragen Hostnamen und Zugangsdaten (A20/L115).
 */
static void
var_retry_key (char * dst, const char * buf, uint_fast8_t idlen)
{
    uint_fast8_t    i;

    for (i = 0; i < idlen && i < VAR_RETRY_KEY_LEN && buf[i]; i++)
    {
        dst[i] = buf[i];
    }

    dst[i] = '\0';
}

static uint_fast8_t
var_retry_hex2 (const char * p)                                 // zwei Hexziffern, wie sie jedes Kommando fuehrt
{
    uint_fast8_t    i;
    uint_fast8_t    sum = 0;

    for (i = 0; i < 2; i++)
    {
        sum <<= 4;

        if (p[i] >= '0' && p[i] <= '9')
        {
            sum += p[i] - '0';
        }
        else if (p[i] >= 'a' && p[i] <= 'f')
        {
            sum += p[i] - 'a' + 10;
        }
        else if (p[i] >= 'A' && p[i] <= 'F')
        {
            sum += p[i] - 'A' + 10;
        }
    }

    return sum;
}

/* Lohnt es, dieses Kommando vorzumerken? (Design 3.3)
 *
 * Die Ausschlussliste steht hier an EINER Stelle und nicht verstreut an den Aufrufstellen, damit
 * die Begruendung neben der Entscheidung steht:
 *
 *   T...   die aktuelle Zeit. Idempotent, aber VERALTEND -- eine zwei Sekunden alte Uhrzeit
 *          nachzusenden ist nicht falsch, nur sinnlos. Sie kommt im naechsten Zyklus ohnehin.
 *   N..    Uptime (lo/hi), LDR-Rohwert, RTC- und DS18xx-Temperaturindex: dieselbe Lage, zyklisch.
 *
 * Der Grund ist nicht Sparsamkeit, sondern Verdraengung: Vier Plaetze, und die zyklischen Werte
 * kaemen im Stoerfall als Erste und immer wieder. Sie wuerden genau die Werte verdraengen, fuer die
 * die Liste da ist -- die EINMALIG angekuendigten aus dem Nachsendestoss nach einem ESP-Neustart
 * (HARDWARE_CONFIGURATION, Zeitzone, Helligkeit, Overlays; L42, L103, L205).
 */
static uint_fast8_t
var_retry_is_worth_it (const char * buf)
{
    uint_fast8_t    idx;

    if (buf[0] == 'T')                                          // aktuelle Zeit und Datum
    {
        return 0;
    }

    if (buf[0] == 'N' && buf[1] && buf[2])
    {
        idx = var_retry_hex2 (buf + 1);

        if (idx == UPTIME_SECONDS_LO_NUM_VAR || idx == UPTIME_SECONDS_HI_NUM_VAR ||
            idx == LDR_RAW_VALUE_NUM_VAR ||
            idx == RTC_TEMP_INDEX_NUM_VAR || idx == DS18XX_TEMP_INDEX_NUM_VAR)
        {
            return 0;
        }
    }

    return 1;
}

/* Regel der Kennung (Design 3.4, Risiko R-1) -- der Teil, ohne den diese ganze Aenderung abzulehnen
 * waere.
 *
 * Die Kennung sind die fuehrenden Zeichen, die die VARIABLE bezeichnen, ohne ihren Wert:
 *
 *   var_send_byte/_short/_string   <id><idx:2>      idlen = strlen (id) + 2
 *   var_send_num_variable          N<idx:2>         3
 *   var_send_num8_array            n<idx:2><i:2>    5
 *   var_send_str_variable          S<idx:2>         3
 *   var_send_tm_variable           T<idx:2>         3
 *   var_send_dsp_color_variable    DC<idx:2>        4
 *   Nacht-/Alarmtabellen           <t|a|l><idx:2>   3
 *   IR-Code                        I<idx:2>         3
 *
 * Geraeumt wird VOR jedem Senden und NACH jeder eingetroffenen Quittung. Ohne beides baute die
 * Nachsendung genau den Schaden ein, gegen den sie antritt: Ein Kommando laeuft in den Timeout und
 * wird vorgemerkt, zwei Sekunden spaeter setzt der Nutzer denselben Wert neu und das gelingt --
 * und danach schriebe die Nachsendung den ALTEN Wert. Ein falscher Wert, der gueltig aussieht, ist
 * genau die Schadensform aus L205 und schlimmer als ein fehlender.
 */
static void
var_retry_purge (const char * buf, uint_fast8_t idlen)
{
    uint_fast8_t    i;

    if (! idlen)
    {
        return;
    }

    for (i = 0; i < VAR_RETRY_SLOTS; i++)
    {
        if (var_retry_slots[i].cmd[0] && var_retry_slots[i].idlen == idlen &&
            ! strncmp (var_retry_slots[i].cmd, buf, idlen))
        {
            var_retry_slots[i].cmd[0] = '\0';
        }
    }
}

/* Ein Kommando vormerken. Rueckgabe 1, wenn es in der Liste steht -- der Aufrufer meldet sonst
 * "vorgemerkt", wo nichts vorgemerkt wurde.
 */
static uint_fast8_t
var_retry_queue (const char * buf, uint_fast8_t idlen, uint_fast8_t attempts, uint32_t due)
{
    char            key[VAR_RETRY_KEY_LEN + 1];
    uint_fast8_t    i;

    if (! idlen || ! var_retry_is_worth_it (buf))
    {
        return 0;
    }

    if (strlen (buf) > VAR_RETRY_CMD_LEN)
    {
        if (var_retry_toolong_cnt < 0xFFFF)                     // saettigend
        {
            var_retry_toolong_cnt++;
        }

        return 0;
    }

    if (attempts >= VAR_RETRY_MAX_ATTEMPTS)
    {
        if (var_retry_gaveup_cnt < 0xFFFF)                      // saettigend
        {
            var_retry_gaveup_cnt++;
        }

        var_retry_key (key, buf, idlen);
        log_printf ("var retry: aufgegeben %s nach %d Versuchen\r\n", key, (int) attempts);
        return 0;
    }

    var_retry_purge (buf, idlen);                               // der juengere Wert gilt, siehe oben

    for (i = 0; i < VAR_RETRY_SLOTS; i++)
    {
        if (! var_retry_slots[i].cmd[0])
        {
            strcpy (var_retry_slots[i].cmd, buf);
            var_retry_slots[i].idlen    = idlen;
            var_retry_slots[i].attempts = attempts;
            var_retry_slots[i].due      = due;
            return 1;
        }
    }

    /* Dass die Liste volllaeuft, ist kein Fehler, sondern eine Aussage: Die Nachsendung auf
     * Kommandoebene ist ueberfordert, der Zustand der Gegenstelle ist zweifelhaft. Genau dann waere
     * ein Vollabgleich das richtige Mittel -- der ist bewusst nicht Teil dieser Aenderung.
     */
    if (var_retry_dropped_cnt < 0xFFFF)                         // saettigend
    {
        var_retry_dropped_cnt++;
    }

    if (! var_retry_full_logged || uptime - var_retry_full_log_last >= VAR_RETRY_FULL_LOG_SEC)
    {
        var_retry_full_logged   = 1;
        var_retry_full_log_last = uptime;
        log_printf ("var retry: Liste voll, %d verworfen\r\n", (int) var_retry_dropped_cnt);
    }

    return 0;
}

/* Das Kommando, auf dessen Quittung gerade gewartet wird. Zeigt in den Stapelrahmen des wartenden
 * Aufrufs und ist nur waehrend der Warteschleife gueltig.
 *
 * Wozu: Die Warteschleife ruft schedule_esp8266_messages() und fuehrt eintreffende Kommandos damit
 * verschachtelt aus. Setzt eines davon DIESELBE Variable, geht der neue Wert sofort raus -- und der
 * Wert, auf den hier noch gewartet wird, ist damit veraltet. Er darf dann NICHT mehr vorgemerkt
 * werden, sonst ueberschriebe die Nachsendung spaeter den juengeren Wert (R-1). Die Liste allein
 * kann das nicht zeigen: Ein verschachtelter Aufruf wartet nie und merkt deshalb auch nichts vor.
 */
static const char * var_send_cur_buf = (const char *) 0;
static uint_fast8_t var_send_cur_idlen = 0;
static uint_fast8_t var_send_superseded = 0;

/* Rueckgabe: 1, wenn der ESP das Kommando quittiert hat (Punkt). 0 sonst -- bei Zeitueberschreitung,
 * bei Ablehnung (!v) und im verschachtelten Fall, der gar nicht erst wartet.
 */
static uint_fast8_t
var_send_buf (char * buf, uint_fast8_t idlen)
{
    char            crc_buf[6];                                 // "*" + 4 Hexziffern + Nullbyte, direkt auf die UART
    uint32_t        start_uptime;
    uint_fast8_t    attempts;
    uint_fast8_t    got_answer;                                 // Punkt ODER !v: die Bruecke lebt
    uint_fast8_t    applied;                                    // nur Punkt: der Wert ist gesetzt
    uint_fast8_t    timed_out;
    uint_fast8_t    msg_rtc;

    attempts = var_retry_attempts_in;                           // vom Drain gesetzt, gilt genau fuer diesen Aufruf
    var_retry_attempts_in = 0;

    var_retry_purge (buf, idlen);                               // Regel 1: der juengere Wert verdraengt den vorgemerkten

    if (var_send_nested && var_send_cur_idlen && idlen == var_send_cur_idlen &&
        ! strncmp (buf, var_send_cur_buf, idlen))
    {
        var_send_superseded = 1;                                // der wartende Aufruf traegt jetzt den aelteren Wert
    }

    esp8266_uart_puts ("var ");
    esp8266_uart_puts (buf);

    /* Die Pruefsumme NUR, wenn der ESP sie gemeldet hat (Design 6.4, Risiko R-3). Ein ESP ohne
     * Pruefsummenverstaendnis speicherte "meinhost*c328" als Hostnamen -- ein stiller falscher Wert
     * und damit genau der Schaden, gegen den die Pruefsumme antritt. OTA-Rueckrollen ist hier
     * Routine, der Fall ist nicht theoretisch. Ohne die Meldung verhaelt sich alles wie vorher.
     */
    if (esp8266.cap_var_crc)
    {
        sprintf (crc_buf, "*%04x", (unsigned int) var_crc (buf));
        esp8266_uart_puts (crc_buf);
    }

    esp8266_uart_puts ("\r\n");
    esp8266_uart_flush ();

    debug_log_printf ("var_send: %s\r\n", buf);
    debug_log_flush ();

    /* Die Warteschleife unten ruft schedule_esp8266_messages() selbst auf und fuehrt
     * eintreffende Kommandos damit verschachtelt aus. Ruft eines davon wieder hier
     * herein, darf NICHT erneut gewartet werden -- sonst verschachtelt sich das
     * beliebig tief. Das Kommando ist oben bereits rausgegangen, nur die
     * Quittungspruefung entfaellt.
     *
     * var_send_busy taugt dafuer nicht: main.c setzt es bei jedem ESP8266_OK zurueck,
     * auch bei dem eines verschachtelten Kommandos.
     *
     * Vorgemerkt wird hier NICHT: Ohne Quittungspruefung bemerkt niemand, dass das Kommando
     * fehlt. Gezaehlt wird es (v=<timeouts>/<verschachtelt>); aufgeloest wuerde dieser Fall erst
     * durch den vertagten Vollabgleich, nicht durch die Nachsendeliste.
     */
    if (var_send_nested)
    {
        if (var_send_nested_cnt < 0xFFFF)                       // saettigend: 65535 heisst "mindestens 65535"
        {
            var_send_nested_cnt++;
        }

        return 0;
    }

    var_send_nested = 1;
    var_send_busy = 1;
    start_uptime = uptime;
    got_answer = 0;
    applied = 0;
    timed_out = 0;

    var_send_cur_buf     = buf;
    var_send_cur_idlen   = idlen;
    var_send_superseded  = 0;

    if (! var_send_reload_armed)                                // erster wartender Aufruf seit dem Loopkopf: Budget beginnt hier
    {
        var_send_reload_armed = 1;
        var_send_reload_start = start_uptime;
    }

    /* Frueher stand hier eine Schleife ohne jede Abbruchbedingung. Blieb die Quittung
     * aus, kehrte der Aufrufer nie zurueck. Am 30.09.2026 zweimal reproduziert: Die
     * Uhr blieb mitten in set_display_power() stehen und lief bis zum manuellen Reset
     * nicht weiter.
     *
     * Die Abbruchbedingung stand seither im Schleifenkopf und steht jetzt im Rumpf: Nur
     * so ist nach der Schleife unterscheidbar, WARUM sie verlassen wurde -- mit Quittung,
     * mit Ablehnung oder nach Zeitueberschreitung. Diese Unterscheidung traegt den Reload
     * unten und die Nachsendung.
     */
    while (1)
    {
        msg_rtc = schedule_esp8266_messages ();

        if (msg_rtc == ESP8266_OK)
        {
            got_answer = 1;                                     // Bruecke hat geantwortet
            applied    = 1;                                     // und den Wert uebernommen
            break;
        }

        /* Ablehnung: Die Zeile kam verstuemmelt an, der ESP hat sie NICHT angewandt (Design 6.3).
         * Hier wird nicht erneut gewartet -- die Bruecke lebt ja, es fehlt nur eine heile Zeile.
         * Vorgemerkt wird sofort faellig, gesendet wird im naechsten Hauptloop-Durchlauf.
         */
        if (msg_rtc == ESP8266_NAK)
        {
            got_answer = 1;
            break;
        }

        if (uptime - start_uptime >= VAR_SEND_TIMEOUT_SEC)
        {
            timed_out = 1;
            break;                                              // got_answer bleibt 0
        }
    }

    var_send_busy = 0;
    var_send_nested = 0;
    var_send_cur_buf = (const char *) 0;
    var_send_cur_idlen = 0;

    if (applied)
    {
        var_retry_purge (buf, idlen);                           // Regel 2: bestaetigt gesetzt, nichts mehr nachzusenden
    }
    else if (! var_send_superseded)
    {
        uint_fast8_t    queued;

        queued = var_retry_queue (buf, idlen, attempts, got_answer ? uptime : uptime + VAR_RETRY_DELAY_SEC);

        if (timed_out)
        {
            if (var_send_timeout_cnt < 0xFFFF)                  // saettigend: 65535 heisst "mindestens 65535"
            {
                var_send_timeout_cnt++;
            }

            /* Hier stand "weiter ohne" -- nach der Nachsendeliste waere das eine Falschaussage.
             * Der Wert bleibt wie er war (A20/L115 ist nicht Teil dieser Aenderung), nur das Wort
             * sagt jetzt, was wirklich geschieht: entweder vorgemerkt oder eben doch verworfen.
             */
            log_printf ("var_send_buf: keine Quittung nach %ds, %s: %s\r\n",
                        VAR_SEND_TIMEOUT_SEC, queued ? "vorgemerkt" : "verworfen", buf);
        }
    }
    else if (timed_out)
    {
        if (var_send_timeout_cnt < 0xFFFF)                      // saettigend
        {
            var_send_timeout_cnt++;
        }

        log_printf ("var_send_buf: keine Quittung nach %ds, inzwischen neuer Wert: %s\r\n",
                    VAR_SEND_TIMEOUT_SEC, buf);
    }

    /* Watchdog NUR bei eingetroffener Antwort bedienen, niemals nach dem Timeout.
     *
     * var_send_all_variables() sendet rund 194 Kommandos (ueber 450 mit vollen Overlays),
     * jedes bis zu VAR_SEND_TIMEOUT_SEC lang blockierend, und im ganzen Pfad stand bisher
     * kein einziger watchdog_reload() (BEFUNDE.md, L85). Antwortet die Bruecke langsam,
     * aber sie antwortet, setzt der Watchdog mitten im Vollabgleich zurueck, obwohl nichts
     * kaputt ist -- der ESP behaelt dann einen halben Variablensatz.
     *
     * Bedingungslos darf der Reload aber NICHT stehen: Bei toter Bruecke ist der
     * Watchdog-Reset die einzige Selbstheilung. Am 02.10.2026 hat er die Uhr nach sieben
     * Sekunden wieder ins Leben gebracht (L25). Ein Reload nach dem Timeout machte daraus
     * wieder ein stilles Steckenbleiben. Daran aendert die Nachsendeliste nichts: Sie wird
     * NUR bei Antwort oder gar nicht bedient, und bei toter Bruecke greift weiterhin der Reset.
     *
     * Entschieden wird ueber got_answer und nicht ueber applied: Ein abgelehntes Kommando (!v)
     * ist kein Erfolg, aber es ist ein Lebenszeichen der Bruecke -- und ein Reset dafuer waere
     * falsch. ESP8266_OK entsteht nur aus dem Punkt (esp8266.c), den der ESP unmittelbar nach
     * jedem var-Kommando sendet. Die drei unaufgeforderten "OK ..."-Zeilen seines Bootlaufs
     * liefern seit specs/bruecke Design 1 ESP8266_STATUS und setzen got_answer nicht.
     *
     * Der verschachtelte Fall erreicht diese Stelle gar nicht: Er kehrt oben bei
     * var_send_nested zurueck, hat also nie gewartet. Ein Reload dort waere keiner
     * "nach Quittung", sondern einer ohne jede Aussage; der aeussere Aufruf erledigt ihn.
     *
     * Seit specs/bruecke Design 2 traegt der Reload zusaetzlich ein Zeitbudget je
     * Hauptloop-Durchlauf. Die Quittung allein genuegt nicht mehr: Eine sporadisch antwortende
     * Bruecke erfuellt sie dauernd und hielte den Hauptloop damit bis zu zehn Minuten am Leben,
     * ohne dass die Uhr weiterlaeuft. Nach VAR_SEND_RELOAD_BUDGET_SEC greift der Watchdog wieder
     * wie vor 9eb6dd9. Der Nullpunkt wird am Kopf des Hauptloops geloescht, der Vergleich ist
     * vorzeichenlos und ueberlebt den Umlauf von uptime.
     */
    if (got_answer && uptime - var_send_reload_start < VAR_SEND_RELOAD_BUDGET_SEC)
    {
        watchdog_reload ();
    }

    return applied;
}

/* Nachsendung, getaktet vom Hauptloop -- hoechstens EIN Kommando je Durchlauf (Design 3.5).
 *
 * Dasselbe Muster wie der IR-Abzug in main.c, und aus demselben Grund: Zwischen zwei Kommandos
 * liegt dadurch garantiert der watchdog_reload() vom Kopf des Hauptloops, und es braucht keine
 * neue Aufrufstelle (AK6, Guardrail S7). Der laengste zusammenhaengende Stillstand bleibt bei
 * VAR_SEND_TIMEOUT_SEC = 3 s, weil nie zwei Versuche in demselben Durchlauf liegen koennen.
 *
 * Rueckgabe 1, wenn ein Versuch unternommen wurde.
 */
uint_fast8_t
var_retry_drain (void)
{
    char            cmd[VAR_RETRY_CMD_LEN + 1];
    char            key[VAR_RETRY_KEY_LEN + 1];
    uint_fast8_t    i;
    uint_fast8_t    pick = VAR_RETRY_SLOTS;
    uint_fast8_t    idlen;
    uint_fast8_t    attempts;

    if (uptime - var_retry_last_attempt < VAR_RETRY_SPACING_SEC)
    {
        return 0;
    }

    for (i = 0; i < VAR_RETRY_SLOTS; i++)
    {
        if (! var_retry_slots[i].cmd[0])
        {
            continue;
        }

        if ((uint32_t) (uptime - var_retry_slots[i].due) >= 0x80000000UL)       // noch nicht faellig, umlaufsicher
        {
            continue;
        }

        if (pick == VAR_RETRY_SLOTS ||
            (uint32_t) (var_retry_slots[i].due - var_retry_slots[pick].due) >= 0x80000000UL)
        {
            pick = i;                                           // der aelteste faellige Eintrag zuerst
        }
    }

    if (pick == VAR_RETRY_SLOTS)
    {
        return 0;
    }

    /* Der Eintrag wird VOR dem Senden aus der Liste genommen und nur dann zurueckgelegt, wenn er
     * danach noch gebraucht wird -- var_send_buf() legt ihn selbst wieder ab. Grund ist die
     * Wiedereintrittsfalle: Die Warteschleife fuehrt eintreffende Kommandos aus, und eines davon
     * kann dieselbe Variable neu setzen. Dann traegt der Eintrag hier den aelteren Wert und darf
     * nicht zurueck (var_send_superseded).
     */
    strcpy (cmd, var_retry_slots[pick].cmd);
    idlen    = var_retry_slots[pick].idlen;
    attempts = var_retry_slots[pick].attempts;
    var_retry_slots[pick].cmd[0] = '\0';

    var_retry_last_attempt = uptime;
    var_retry_attempts_in  = attempts + 1;

    if (var_send_buf (cmd, idlen))
    {
        if (var_retry_ok_cnt < 0xFFFF)                          // saettigend
        {
            var_retry_ok_cnt++;
        }

        var_retry_key (key, cmd, idlen);
        log_printf ("var retry: ok %s nach %d Versuchen\r\n", key, (int) (attempts + 1));
    }

    return 1;
}

/* A32: Ende des Abschnitts, den der Tischpruefstand einbindet (Anfang: #define VAR_SEND_TIMEOUT_SEC). */

static void
var_send_byte (const char * id, uint_fast32_t var, uint_fast8_t value)
{
    char            buf[32];

    sprintf (buf, "%s%02x%02x", id, (int) var, value);
    var_send_buf (buf, (uint_fast8_t) (strlen (id) + 2));                 // Kennung: <id><idx:2>
}

static void
var_send_short (const char * id, uint_fast32_t var, uint_fast16_t value)
{
    char            buf[32];

    sprintf (buf, "%s%02x%04x", id, (int) var, value & 0xFFFF);
    var_send_buf (buf, (uint_fast8_t) (strlen (id) + 2));                 // Kennung: <id><idx:2>
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send a named string to ESP8266: <id><var:2><value>
 *
 * Befund L86: Hier stand sprintf in einen 160-Byte-Stackpuffer ohne jede Laengenpruefung des
 * uebergebenen Werts. Heute speist keine Quelle mehr als rund 64 Zeichen ein, der Fehler war
 * also latent -- ein laengerer Wert haette den Stack ueberschrieben.
 *
 * snprintf kappt. Genau dieses Kappen darf aber NICHT stillschweigend passieren: Eine gekuerzte
 * var-Zeile ist fuer den ESP syntaktisch gueltig und wird dort als richtiger Wert uebernommen.
 * Aus einem Absturz wuerde so eine stille Verfaelschung, und die ist hier die schlimmere Sorte.
 * Im Ueberlauffall geht deshalb gar nichts raus, und der Fall wird gemeldet -- ueber log_printf,
 * nicht ueber debug_log_printf: Letzteres ist in der ausgelieferten Firmware ein leeres Makro
 * (Befund L87, log.h:23-31, DEBUG wird nirgends definiert).
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static void
var_send_string (const char * id, uint_fast32_t var, const char * value)
{
    char            buf[160];
    int             len;

    len = snprintf (buf, sizeof (buf), "%s%02x%s", id, (int) var, value);

    if (len < 0 || (size_t) len >= sizeof (buf))
    {
        log_printf ("var_send_string: %s%02x zu lang (%d von max %d Zeichen), nicht gesendet\r\n",
                    id, (int) var, len, (int) sizeof (buf) - 1);
        return;
    }

    var_send_buf (buf, (uint_fast8_t) (strlen (id) + 2));                 // Kennung: <id><idx:2>
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send a numeric variable to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static void
var_send_num_variable (NUM_VARIABLE var, unsigned int value)
{
    char            buf[32];

    if (var < MAX_NUM_VARIABLES)
    {
        sprintf (buf, "N%02x%02x%02x", (int) var, value & 0xFF, (value >> 8) & 0xFF);
        var_send_buf (buf, 3);                                            // Kennung: N<idx:2>
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send a numeric array (n * uint8_t) to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static void
var_send_num8_array (NUM8_ARRAY var, uint_fast8_t * p, uint_fast8_t n)
{
    char            buf[32];
    uint_fast8_t    i;

    if (var < MAX_NUM8_ARRAYS)
    {
        for (i = 0; i < n; i++)
        {
            sprintf (buf, "n%02x%02x%02x", var, i, p[i]);
            var_send_buf (buf, 5);                                        // Kennung: n<idx:2><i:2>
        }
    }
}

#if 0 // yet not used
/*--------------------------------------------------------------------------------------------------------------------------------------
 * send a numeric (n * uint16_t) array to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static void
var_send_num16_array (NUM16_ARRAY var, uint_fast16_t * p, uint_fast8_t n)
{
    char            buf[32];
    uint_fast8_t    i;

    if (var < MAX_NUM16_ARRAYS)
    {
        for (i = 0; i < n; i++)
        {
            sprintf (buf, "m%02x%02x%02x%02x", var, i, p[i] & 0xFF, (p[i] >> 8) & 0xFF);
            var_send_buf (buf, 5);                                        // Kennung: m<idx:2><i:2>
        }
    }
}
#endif // 0

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send a string variable to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static void
var_send_str_variable (STR_VARIABLE var, const char * value)
{
    char            buf[160];
    int             len;

    if (var < MAX_STR_VARIABLES && value)
    {
        len = snprintf (buf, sizeof (buf), "S%02x%s", (int) var, value);

        if (len < 0 || (size_t) len >= sizeof (buf))                    // Befund L86, Begruendung siehe var_send_string()
        {
            log_printf ("var_send_str_variable: S%02x zu lang (%d von max %d Zeichen), nicht gesendet\r\n",
                        (int) var, len, (int) sizeof (buf) - 1);
            return;
        }

        var_send_buf (buf, 3);                                            // Kennung: S<idx:2>
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send a tm variable to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static void
var_send_tm_variable (TM_VARIABLE var, TM * tm)
{
    char            buf[32];

    if (var < MAX_TM_VARIABLES)
    {
        sprintf (buf, "T%02x%04d%02d%02d%02d%02d%02d", (int) var, tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        var_send_buf (buf, 3);                                            // Kennung: T<idx:2>
    }
}

static void
var_send_uptime (void)
{
    uint32_t current_uptime = uptime;

    var_send_num_variable (UPTIME_SECONDS_LO_NUM_VAR, current_uptime & 0xFFFF);
    var_send_num_variable (UPTIME_SECONDS_HI_NUM_VAR, (current_uptime >> 16) & 0xFFFF);
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send a dsp color variable to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static void
var_send_dsp_color_variable (DSP_COLOR_VARIABLE var, DSP_COLORS * dsp_colors)
{
    char buf[32];

#if DSP_USE_SK6812_RGBW == 1
    sprintf (buf, "DC%02x%02x%02x%02x%02x", (int) var, dsp_colors->red, dsp_colors->green, dsp_colors->blue, dsp_colors->white);
#else
    sprintf (buf, "DC%02x%02x%02x%02x00", (int) var, dsp_colors->red, dsp_colors->green, dsp_colors->blue);
#endif
    var_send_buf (buf, 4);                                                // Kennung: DC<idx:2>
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send an animation variable to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static void
var_send_display_animation_name (DISPLAY_ANIMATION_VARIABLE var, const char * name)
{
    if (var < MAX_DISPLAY_ANIMATION_VARIABLES)
    {
        var_send_string ("AN", (uint_fast32_t) var, name);
    }
}

static void
var_send_display_animation_deceleration (DISPLAY_ANIMATION_VARIABLE var, uint_fast8_t deceleration)
{
    if (var < MAX_DISPLAY_ANIMATION_VARIABLES)
    {
        var_send_byte ("AD", (uint_fast32_t) var, deceleration);
    }
}

static void
var_send_display_animation_default_deceleration (DISPLAY_ANIMATION_VARIABLE var, uint_fast8_t default_deceleration)
{
    if (var < MAX_DISPLAY_ANIMATION_VARIABLES)
    {
        var_send_byte ("AE", (uint_fast32_t) var, default_deceleration);
    }
}

static void
var_send_display_animation_flags (DISPLAY_ANIMATION_VARIABLE var, uint_fast8_t flags)
{
    if (var < MAX_DISPLAY_ANIMATION_VARIABLES)
    {
        var_send_byte ("AF", (uint_fast32_t) var, flags);
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send a color animation variable to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static void
var_send_color_animation_name (COLOR_ANIMATION_VARIABLE var, const char * name)
{
    if (var < MAX_COLOR_ANIMATION_VARIABLES)
    {
        var_send_string ("CN", (uint_fast32_t) var, name);
    }
}

static void
var_send_color_animation_deceleration (COLOR_ANIMATION_VARIABLE var, uint_fast8_t deceleration)
{
    if (var < MAX_COLOR_ANIMATION_VARIABLES)
    {
        var_send_byte ("CD", (uint_fast32_t) var, deceleration);
    }
}

static void
var_send_color_animation_default_deceleration (COLOR_ANIMATION_VARIABLE var, uint_fast8_t default_deceleration)
{
    if (var < MAX_COLOR_ANIMATION_VARIABLES)
    {
        var_send_byte ("CE", (uint_fast32_t) var, default_deceleration);
    }
}

static void
var_send_color_animation_flags (COLOR_ANIMATION_VARIABLE var, uint_fast8_t flags)
{
    if (var < MAX_COLOR_ANIMATION_VARIABLES)
    {
        var_send_byte ("CF", (uint_fast32_t) var, flags);
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send a ambilight mode variable to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static void
var_send_ambilight_mode_name (AMBILIGHT_MODE_VARIABLE var, const char * name)
{
    if (var < MAX_AMBILIGHT_MODE_VARIABLES)
    {
        var_send_string ("MN", (uint_fast32_t) var, name);
    }
}

static void
var_send_ambilight_mode_deceleration (AMBILIGHT_MODE_VARIABLE var, uint_fast8_t deceleration)
{
    if (var < MAX_AMBILIGHT_MODE_VARIABLES)
    {
        var_send_byte ("MD", (uint_fast32_t) var, deceleration);
    }
}

static void
var_send_ambilight_mode_default_deceleration (AMBILIGHT_MODE_VARIABLE var, uint_fast8_t default_deceleration)
{
    if (var < MAX_AMBILIGHT_MODE_VARIABLES)
    {
        var_send_byte ("ME", (uint_fast32_t) var, default_deceleration);
    }
}

static void
var_send_ambilight_mode_flags (AMBILIGHT_MODE_VARIABLE var, uint_fast8_t flags)
{
    if (var < MAX_AMBILIGHT_MODE_VARIABLES)
    {
        var_send_byte ("MF", (uint_fast32_t) var, flags);
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send overlay data to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static void
var_send_n_overlays (uint_fast8_t n_overlays)
{
    var_send_num_variable (OVERLAY_N_OVERLAYS_NUM_VAR, n_overlays);
}

static void
var_send_overlay_type (uint32_t var_idx, uint_fast8_t type)
{
    if (var_idx < overlay.n_overlays)
    {
        var_send_byte ("OT", (uint_fast32_t) var_idx, type);
    }
}

static void
var_send_overlay_interval (uint32_t var_idx, uint_fast8_t interval)
{
    if (var_idx < overlay.n_overlays)
    {
        var_send_byte ("OI", (uint_fast32_t) var_idx, interval);
    }
}

static void
var_send_overlay_duration (uint32_t var_idx, uint_fast8_t duration)
{
    if (var_idx < overlay.n_overlays)
    {
        var_send_byte ("OD", var_idx, duration);
    }
}

static void
var_send_overlay_date_code (uint32_t var_idx, uint_fast8_t date_code)
{
    if (var_idx < overlay.n_overlays)
    {
        var_send_byte ("OC", var_idx, date_code);
    }
}

static void
var_send_overlay_date_start (uint32_t var_idx, uint_fast16_t date_start)
{
    if (var_idx < overlay.n_overlays)
    {
        var_send_short ("OS", var_idx, date_start);
    }
}

static void
var_send_overlay_days (uint32_t var_idx, uint_fast8_t days)
{
    if (var_idx < overlay.n_overlays)
    {
        var_send_byte ("OY", var_idx, days);
    }
}

static void
var_send_overlay_text (uint32_t var_idx, const char * text)
{
    if (var_idx < overlay.n_overlays)
    {
        var_send_string ("ON", var_idx, text);
    }
}

static void
var_send_overlay_flags (uint32_t var_idx, uint_fast8_t flags)
{
    if (var_idx < overlay.n_overlays)
    {
        var_send_byte ("OF", var_idx, flags);
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send night time variables to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static void
var_send_night_time (NIGHT_TIME_VARIABLE var, uint_fast16_t minutes, uint_fast8_t flags)
{
    char buf[32];

    if (var < MAX_NIGHT_TIME_VARIABLES)
    {
        sprintf (buf, "t%02x%02x%02x%02x", (int) var, minutes & 0xFF, (minutes >> 8) & 0xFF, flags);
        var_send_buf (buf, 3);                                            // Kennung: t<idx:2>
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send ambilight night time variables to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static void
var_send_ambilight_night_time (NIGHT_TIME_VARIABLE var, uint_fast16_t minutes, uint_fast8_t flags)
{
    char buf[32];

    if (var < MAX_NIGHT_TIME_VARIABLES)
    {
        sprintf (buf, "a%02x%02x%02x%02x", (int) var, minutes & 0xFF, (minutes >> 8) & 0xFF, flags);
        var_send_buf (buf, 3);                                            // Kennung: a<idx:2>
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send alarm time variables to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static void
var_send_alarm_time (ALARM_TIME_VARIABLE var, uint_fast16_t minutes, uint_fast8_t flags)
{
    char buf[32];

    if (var < MAX_ALARM_TIME_VARIABLES)
    {
        sprintf (buf, "l%02x%02x%02x%02x", (int) var, minutes & 0xFF, (minutes >> 8) & 0xFF, flags);
        var_send_buf (buf, 3);                                            // Kennung: l<idx:2>
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send global variables to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
void
var_send_use_rgbw (void)
{
#if DSP_USE_SK6812_RGBW == 1
    var_send_num_variable (DISPLAY_USE_RGBW_NUM_VAR, 1);
#else
    var_send_num_variable (DISPLAY_USE_RGBW_NUM_VAR, 0);
#endif
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send global variables to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static void
var_send_eep_is_up (void)
{
    var_send_num_variable (EEPROM_IS_UP_NUM_VAR, eep_is_up);
}

static void
var_send_hardware_configuration (void)
{
    var_send_num_variable (HARDWARE_CONFIGURATION_NUM_VAR, gmain.hardware_configuration);
}

static void
var_send_rtc_is_up (void)
{
    var_send_num_variable (RTC_IS_UP_NUM_VAR, grtc.rtc_is_up);
}

void
var_send_display_power (void)
{
    var_send_num_variable (DISPLAY_POWER_NUM_VAR, display.display_power_is_on);
}

static void
var_send_display_ambilight_power (void)
{
    var_send_num_variable (DISPLAY_AMBILIGHT_POWER_NUM_VAR, display.ambilight_power_is_on);
}

void
var_send_display_mode (void)
{
    var_send_num_variable (DISPLAY_MODE_NUM_VAR, display.display_mode);
}

void
var_send_display_brightness (void)
{
    var_send_num_variable (DISPLAY_BRIGHTNESS_NUM_VAR, display.display_brightness);
}

static void
var_send_display_flags (void)
{
    var_send_num_variable (DISPLAY_FLAGS_NUM_VAR, display.display_flags);
}

void
var_send_display_automatic_brightness_active (void)
{
    var_send_num_variable (DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE_NUM_VAR, display.automatic_brightness);
}

void
var_send_animation_mode (void)
{
    var_send_num_variable (ANIMATION_MODE_NUM_VAR, display.animation_mode);
}

void
var_send_ambilight_mode (void)
{
    var_send_num_variable (AMBILIGHT_MODE_NUM_VAR, display.ambilight_mode);
}

static void
var_send_ambilight_leds (void)
{
    var_send_num_variable (AMBILIGHT_LEDS_NUM_VAR, display.ambilight_leds);
}

static void
var_send_ambilight_offset (void)
{
    var_send_num_variable (AMBILIGHT_OFFSET_NUM_VAR, display.ambilight_led_offset);
}

void
var_send_ambilight_brightness (void)
{
    var_send_num_variable (AMBILIGHT_BRIGHTNESS_NUM_VAR, display.ambilight_brightness);
}

void
var_send_color_animation_mode (void)
{
    var_send_num_variable (COLOR_ANIMATION_MODE_NUM_VAR, display.color_animation_mode);
}

void
var_send_ldr_raw_value (void)
{
    var_send_num_variable (LDR_RAW_VALUE_NUM_VAR, ldr.ldr_raw_value);
}

void
var_send_ldr_min_value (void)
{
    var_send_num_variable (LDR_MIN_VALUE_NUM_VAR, ldr.ldr_min_value);
}

void
var_send_ldr_max_value (void)
{
    var_send_num_variable (LDR_MAX_VALUE_NUM_VAR, ldr.ldr_max_value);
}

static void
var_send_timezone (void)
{
    uint16_t    tz;

    if (timeserver.timezone < 0)
    {
        tz = -timeserver.timezone;
        tz |= 0x100;
    }
    else
    {
        tz = timeserver.timezone;
    }

    if (timeserver.observe_summertime)
    {
        tz |= 0x200;
    }

    var_send_num_variable (TIMEZONE_NUM_VAR, tz);
}

static void
var_send_ds18xx_is_up (void)
{
    var_send_num_variable (DS18XX_IS_UP_NUM_VAR, ds18xx.is_up);
}

void
var_send_rtc_temp_index (void)
{
    var_send_num_variable (RTC_TEMP_INDEX_NUM_VAR, grtc.rtc_temperature_index);
}

static void
var_send_rtc_temp_correction (void)
{
    // Auf dem Draht steht weiterhin genau ein Zweierkomplement-Byte. Die Maske haelt das
    // Vorzeichen aus dem High-Byte heraus, sonst aendert sich das Protokoll gegenueber dem ESP.
    var_send_num_variable (RTC_TEMP_CORRECTION_NUM_VAR, (uint8_t) grtc.rtc_temp_correction);
}

void
var_send_ds18xx_temp_index (void)
{
    var_send_num_variable (DS18XX_TEMP_INDEX_NUM_VAR, gtemp.index);
}

static void
var_send_ds18xx_temp_correction (void)
{
    var_send_num_variable (DS18XX_TEMP_CORRECTION_NUM_VAR, (uint8_t) gtemp.correction);
}

static void
var_send_ticker_deceleration (void)
{
    var_send_num_variable (TICKER_DECELRATION_NUM_VAR, display.ticker_deceleration);
}

static void
var_send_ticker_text (void)
{
    ; // nothing to do
}

void
var_send_version (void)
{
    var_send_str_variable (VERSION_STR_VAR, VERSION);
}

void
var_send_eep_version (void)
{
    var_send_str_variable (EEPROM_VERSION_STR_VAR, gmain.eep_version);
}

static void
var_send_esp8266_version (void)
{
    // nothing to do
}

void
var_send_timeserver (void)
{
    var_send_str_variable (TIMESERVER_STR_VAR, timeserver.timeserver);
}

void
var_send_weather_appid (void)
{
    var_send_str_variable (WEATHER_APPID_STR_VAR, weather.appid);
}

void
var_send_weather_city (void)
{
    var_send_str_variable (WEATHER_CITY_STR_VAR, weather.city);
}

void
var_send_weather_lon (void)
{
    var_send_str_variable (WEATHER_LON_STR_VAR, weather.lon);
}

void
var_send_weather_lat (void)
{
    var_send_str_variable (WEATHER_LAT_STR_VAR, weather.lat);
}

static void
var_send_update_host (void)
{
    var_send_str_variable (UPDATE_HOST_VAR, gmain.update_host);
}

static void
var_send_update_path (void)
{
    var_send_str_variable (UPDATE_PATH_VAR, gmain.update_path);
}

static void
var_send_date_ticker_format (void)
{
    var_send_str_variable (DATE_TICKER_FORMAT_VAR, (char *) display.date_ticker_format);
}

void
var_send_reset_cause (void)
{
    var_send_str_variable (RESET_CAUSE_STR_VAR, main_get_reset_cause ());
}

void
var_send_tm (void)
{
    var_send_tm_variable (CURRENT_TM_VAR, &(gmain.tm));
    var_send_uptime ();
}

void
var_send_display_colors (void)
{
    var_send_dsp_color_variable (DISPLAY_DSP_COLOR_VAR, &(display.display_colors));
}

void
var_send_ambilight_colors (void)
{
    var_send_dsp_color_variable (AMBILIGHT_DSP_COLOR_VAR, &(display.ambilight_colors));
}

void
var_send_ambilight_marker_colors (void)
{
    var_send_dsp_color_variable (AMBILIGHT_MARKER_DSP_COLOR_VAR, &(display.ambilight_marker_colors));
}

void
var_send_display_animations (void)
{
    DISPLAY_ANIMATION_VARIABLE  idx;

    for (idx = 0; idx < ANIMATION_MODES; idx++)
    {
        var_send_display_animation_name (idx, display.animations[idx].name);
        var_send_display_animation_deceleration (idx, display.animations[idx].deceleration);
        var_send_display_animation_default_deceleration (idx, display.animations[idx].default_deceleration);
        var_send_display_animation_flags (idx, display.animations[idx].flags);
    }
}

void
var_send_color_animations (void)
{
    COLOR_ANIMATION_VARIABLE  idx;

    for (idx = 0; idx < COLOR_ANIMATION_MODES; idx++)
    {
        var_send_color_animation_name (idx, display.color_animations[idx].name);
        var_send_color_animation_deceleration (idx, display.color_animations[idx].deceleration);
        var_send_color_animation_default_deceleration (idx, display.color_animations[idx].default_deceleration);
        var_send_color_animation_flags (idx, display.color_animations[idx].flags);
    }
}

void
var_send_ambilight_modes (void)
{
    AMBILIGHT_MODE_VARIABLE  idx;

    for (idx = 0; idx < AMBILIGHT_MODES; idx++)
    {
        var_send_ambilight_mode_name (idx, display.ambilight_modes[idx].name);
        var_send_ambilight_mode_deceleration (idx, display.ambilight_modes[idx].deceleration);
        var_send_ambilight_mode_default_deceleration (idx, display.ambilight_modes[idx].default_deceleration);
        var_send_ambilight_mode_flags (idx, display.ambilight_modes[idx].flags);
    }
}

void
var_send_overlays (void)
{
    uint32_t    idx;

    var_send_n_overlays (overlay.n_overlays);

    for (idx = 0; idx < overlay.n_overlays; idx++)
    {
        var_send_overlay_type (idx, overlay.overlays[idx].type);
        var_send_overlay_interval (idx, overlay.overlays[idx].interval);
        var_send_overlay_duration (idx, overlay.overlays[idx].duration);
        var_send_overlay_date_code (idx, overlay.overlays[idx].date_code);
        var_send_overlay_date_start (idx, overlay.overlays[idx].date_start);
        var_send_overlay_days (idx, overlay.overlays[idx].days);
        var_send_overlay_text (idx, overlay.overlays[idx].text);
        var_send_overlay_flags (idx, overlay.overlays[idx].flags);
    }
}

static void
var_send_night_times (void)
{
    NIGHT_TIME_VARIABLE  idx;

    for (idx = 0; idx < MAX_NIGHT_TIMES; idx++)
    {
        var_send_night_time (idx, night_time[idx].minutes, night_time[idx].flags);
    }
}

static void
var_send_ambilight_night_times (void)
{
    NIGHT_TIME_VARIABLE  idx;

    for (idx = 0; idx < MAX_NIGHT_TIMES; idx++)
    {
        var_send_ambilight_night_time (idx, ambilight_night_time[idx].minutes, ambilight_night_time[idx].flags);
    }
}

static void
var_send_alarm_times (void)
{
    ALARM_TIME_VARIABLE  idx;

    for (idx = 0; idx < MAX_ALARM_TIMES; idx++)
    {
        var_send_alarm_time (idx, alarm_time[idx].minutes, alarm_time[idx].flags);
    }
}

static void
var_send_dimmed_display_colors (void)
{
    var_send_num8_array (DISPLAY_DIMMED_DISPLAY_COLORS, display.dimmed_display_colors, sizeof (display.dimmed_display_colors));
}

static void
var_send_dimmed_ambilight_colors (void)
{
    var_send_num8_array (DISPLAY_DIMMED_AMBILIGHT_COLORS, display.dimmed_ambilight_colors, sizeof (display.dimmed_ambilight_colors));
}

static void
var_send_dfplayer_is_up (void)
{
    var_send_num_variable (DFPLAYER_IS_UP_NUM_VAR, dfplayer.is_up);
}

static void
var_send_dfplayer_version (void)
{
    var_send_num_variable (DFPLAYER_VERSION_NUM_VAR, dfplayer.version);
}

static void
var_send_dfplayer_volume (void)
{
    var_send_num_variable (DFPLAYER_VOLUME_NUM_VAR, dfplayer.volume);
}

static void
var_send_dfplayer_silence_start (void)
{
    var_send_num_variable (DFPLAYER_SILENCE_START_NUM_VAR, dfplayer.silence_start);
}

static void
var_send_dfplayer_silence_stop (void)
{
    var_send_num_variable (DFPLAYER_SILENCE_STOP_NUM_VAR, dfplayer.silence_stop);
}

static void
var_send_dfplayer_mode (void)
{
    var_send_num_variable (DFPLAYER_MODE_NUM_VAR, dfplayer.mode);
}

static void
var_send_dfplayer_bell_flags (void)
{
    var_send_num_variable (DFPLAYER_BELL_FLAGS_NUM_VAR, dfplayer.bell_flags);
}

static void
var_send_dfplayer_speak_cycle (void)
{
    var_send_num_variable (DFPLAYER_SPEAK_CYCLE_NUM_VAR, dfplayer.speak_cycle);
}

#if defined (BLACK_BOARD) // STM32F407
void
var_send_ssd1963_flags (void)
{
    var_send_num_variable (SSD1963_FLAGS_NUM_VAR, ssd1963.flags);
}
#endif

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send one single IR code to ESP8266: I<idx:2><protocol:2><address:4><command:4>
 *
 * 13 Zeichen feste Breite. Alle Werte werden vor dem Formatieren maskiert, denn "%02x" ist
 * eine MINDEST-Breite und keine feste: Genau diese Verwechslung war Befund L66 -- ein Wert
 * ueber 255 erzeugte drei Ziffern, von denen die Gegenseite zwei las. Die Gegenseite liest
 * mit htoi() in fester Breite und koennte eine Verschiebung nicht bemerken.
 *
 * Laenge statisch bekannt: 1 + 2 + 2 + 4 + 4 = 13 Zeichen plus Nullbyte in buf[32]. Befund
 * L86 (sprintf in einen Stackpuffer ohne Laengenpruefung) ist damit nicht beruehrt; er wird
 * hier auch nicht behoben, das ist ein eigener Auftrag.
 *
 * Gerufen wird diese Funktion ausschliesslich aus dem Hauptloop, ein Kommando je Durchlauf,
 * angestossen ueber GET_IR_CODES_RPC_VAR. Sie steht bewusst NICHT in
 * var_send_all_variables() -- siehe dort.
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
void
var_send_ir_code (uint_fast8_t idx)
{
    char            buf[32];
    uint_fast8_t    protocol;
    uint_fast16_t   address;
    uint_fast16_t   command;

    if (remote_ir_get_code (idx, &protocol, &address, &command))
    {
        sprintf (buf, "I%02x%02x%04x%04x",
                 (unsigned int) (idx & 0xFF),
                 (unsigned int) (protocol & 0xFF),
                 (unsigned int) (address & 0xFFFF),
                 (unsigned int) (command & 0xFFFF));
        var_send_buf (buf, 3);                                            // Kennung: I<idx:2>
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send all variables to ESP8266
 *
 * Hier steht bewusst KEIN var_send_ir_code(). Diese Funktion laeuft ohne einen einzigen
 * watchdog_reload() durch (Befund L85) und haengt am Startpfad; die 20 IR-Kommandos gehen
 * deshalb nur auf ausdrueckliche Anforderung raus, getaktet vom Hauptloop.
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
void
var_send_all_variables (void)
{
    var_send_use_rgbw ();
    var_send_eep_is_up ();
    var_send_hardware_configuration ();
    var_send_rtc_is_up ();
    var_send_display_power ();
    var_send_display_ambilight_power ();
    var_send_display_mode ();
    var_send_display_brightness ();
    var_send_display_flags ();
    var_send_display_automatic_brightness_active ();
    var_send_animation_mode ();
    var_send_ambilight_mode ();
    var_send_ambilight_leds ();
    var_send_ambilight_offset ();
    var_send_ambilight_brightness ();
    var_send_color_animation_mode ();
    var_send_ldr_raw_value ();
    var_send_ldr_min_value ();
    var_send_ldr_max_value ();
    var_send_timezone ();
    var_send_ds18xx_is_up ();
    var_send_rtc_temp_index ();
    var_send_rtc_temp_correction ();
    var_send_ds18xx_temp_index ();
    var_send_ds18xx_temp_correction ();
    var_send_ticker_deceleration ();
    var_send_ticker_text ();
    var_send_version ();
    var_send_eep_version ();
    var_send_esp8266_version ();
    var_send_timeserver ();
    var_send_weather_appid ();
    var_send_weather_city ();
    var_send_weather_lon ();
    var_send_weather_lat ();
    var_send_update_host ();
    var_send_update_path ();
    var_send_date_ticker_format ();
    var_send_reset_cause ();

    var_send_tm ();

    var_send_display_colors ();
    var_send_ambilight_colors ();
    var_send_ambilight_marker_colors ();
    var_send_display_animations ();
    var_send_color_animations ();
    var_send_ambilight_modes ();
    var_send_overlays ();
    var_send_night_times ();
    var_send_ambilight_night_times ();
    var_send_dimmed_display_colors ();
    var_send_dimmed_ambilight_colors ();
    var_send_dfplayer_is_up ();
    var_send_dfplayer_version ();
    var_send_dfplayer_volume ();
    var_send_dfplayer_silence_start ();
    var_send_dfplayer_silence_stop ();
    var_send_dfplayer_mode ();
    var_send_dfplayer_bell_flags ();
    var_send_dfplayer_speak_cycle ();
    var_send_alarm_times ();
#if defined (BLACK_BOARD) // STM32F407VE
    var_send_ssd1963_flags ();
#endif
}

