/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * esp8266.c - ESP8266 WLAN routines
 *
 * Copyright (c) 2014-2026 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "delay.h"
#include "display.h"
#include "esp8266.h"
#include "esp8266-config.h"
#include "io.h"
#include "vars.h"                                                       // var_crc(), Teil D
#include "main.h"                                                       // uptime, Teil D

#undef UART_PREFIX
#define UART_PREFIX                     esp8266
#include "uart.h"

#define MSEC(x)                         (x/10)

#define ESP8266_MAX_SEND_CMD_LEN        120

/*--------------------------------------------------------------------------------------------------------------------------------------
 * globals:
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
volatile uint_fast8_t                   esp8266_ten_ms_tick;            // should be set every 10 msec to 1, see IRQ in main.c

ESP8266_GLOBALS                         esp8266;
static uint_fast8_t                     esp8266_uart_ready;

#if defined (DISCO_BOARD)                                               // STM32F4 Discovery Board: RST=PC5 CH_PD=PC4 FLASH=PC3

#define ESP8266_RST_PERIPH_CLOCK_CMD    RCC_AHB1PeriphClockCmd
#define ESP8266_RST_PERIPH              RCC_AHB1Periph_GPIOC
#define ESP8266_RST_PORT                GPIOC
#define ESP8266_RST_PIN                 GPIO_Pin_5

#define ESP8266_CH_PD_PERIPH_CLOCK_CMD  RCC_AHB1PeriphClockCmd
#define ESP8266_CH_PD_PERIPH            RCC_AHB1Periph_GPIOC
#define ESP8266_CH_PD_PORT              GPIOC
#define ESP8266_CH_PD_PIN               GPIO_Pin_4

#define ESP8266_FLASH_PERIPH_CLOCK_CMD  RCC_AHB1PeriphClockCmd
#define ESP8266_FLASH_PERIPH            RCC_AHB1Periph_GPIOC
#define ESP8266_FLASH_PORT              GPIOC
#define ESP8266_FLASH_PIN               GPIO_Pin_3

#elif defined (BLACK_BOARD)                                             // STM32F407VE Black Board: RST=PA4 CH_PD=PA5 FLASH=PA8

#define ESP8266_RST_PERIPH_CLOCK_CMD    RCC_AHB1PeriphClockCmd
#define ESP8266_RST_PERIPH              RCC_AHB1Periph_GPIOA
#define ESP8266_RST_PORT                GPIOA
#define ESP8266_RST_PIN                 GPIO_Pin_4

#define ESP8266_CH_PD_PERIPH_CLOCK_CMD  RCC_AHB1PeriphClockCmd
#define ESP8266_CH_PD_PERIPH            RCC_AHB1Periph_GPIOA
#define ESP8266_CH_PD_PORT              GPIOA
#define ESP8266_CH_PD_PIN               GPIO_Pin_5

#define ESP8266_FLASH_PERIPH_CLOCK_CMD  RCC_AHB1PeriphClockCmd
#define ESP8266_FLASH_PERIPH            RCC_AHB1Periph_GPIOA
#define ESP8266_FLASH_PORT              GPIOA
#define ESP8266_FLASH_PIN               GPIO_Pin_8

#elif defined (NUCLEO_BOARD)                                            // STM32F4xx Nucleo Board: RST=PA7 CH_PD=PA6 FLASH=PA4

#define ESP8266_RST_PERIPH_CLOCK_CMD    RCC_AHB1PeriphClockCmd
#define ESP8266_RST_PERIPH              RCC_AHB1Periph_GPIOA
#define ESP8266_RST_PORT                GPIOA
#define ESP8266_RST_PIN                 GPIO_Pin_7

#define ESP8266_CH_PD_PERIPH_CLOCK_CMD  RCC_AHB1PeriphClockCmd
#define ESP8266_CH_PD_PERIPH            RCC_AHB1Periph_GPIOA
#define ESP8266_CH_PD_PORT              GPIOA
#define ESP8266_CH_PD_PIN               GPIO_Pin_6

#define ESP8266_FLASH_PERIPH_CLOCK_CMD  RCC_AHB1PeriphClockCmd
#define ESP8266_FLASH_PERIPH            RCC_AHB1Periph_GPIOA
#define ESP8266_FLASH_PORT              GPIOA
#define ESP8266_FLASH_PIN               GPIO_Pin_4

#elif defined (BLACKPILL_BOARD)                                         // STM32F4x1 BlackPill Board: RST=PB10 CH_PD=PB3 FLASH=PA4

#define ESP8266_RST_PERIPH_CLOCK_CMD    RCC_AHB1PeriphClockCmd
#define ESP8266_RST_PERIPH              RCC_AHB1Periph_GPIOB
#define ESP8266_RST_PORT                GPIOB
#define ESP8266_RST_PIN                 GPIO_Pin_10

#define ESP8266_CH_PD_PERIPH_CLOCK_CMD  RCC_AHB1PeriphClockCmd
#define ESP8266_CH_PD_PERIPH            RCC_AHB1Periph_GPIOB
#define ESP8266_CH_PD_PORT              GPIOB
#define ESP8266_CH_PD_PIN               GPIO_Pin_3

#define ESP8266_FLASH_PERIPH_CLOCK_CMD  RCC_AHB1PeriphClockCmd
#define ESP8266_FLASH_PERIPH            RCC_AHB1Periph_GPIOA
#define ESP8266_FLASH_PORT              GPIOA
#define ESP8266_FLASH_PIN               GPIO_Pin_4

#elif defined (BLUEPILL_BOARD)                                          // STM32F103 BluePill Board: RST=PA0 CH_PD=PA1 FLASH=PA4

#define ESP8266_RST_PERIPH_CLOCK_CMD    RCC_APB2PeriphClockCmd
#define ESP8266_RST_PERIPH              RCC_APB2Periph_GPIOA
#define ESP8266_RST_PORT                GPIOA
#define ESP8266_RST_PIN                 GPIO_Pin_0

#define ESP8266_CH_PD_PERIPH_CLOCK_CMD  RCC_APB2PeriphClockCmd
#define ESP8266_CH_PD_PERIPH            RCC_APB2Periph_GPIOA
#define ESP8266_CH_PD_PORT              GPIOA
#define ESP8266_CH_PD_PIN               GPIO_Pin_1

#define ESP8266_FLASH_PERIPH_CLOCK_CMD  RCC_APB2PeriphClockCmd
#define ESP8266_FLASH_PERIPH            RCC_APB2Periph_GPIOA
#define ESP8266_FLASH_PORT              GPIOA
#define ESP8266_FLASH_PIN               GPIO_Pin_4

#else
#error STM32 unknown
#endif

#if ESP8266_DEBUG == 1
#undef UART_PREFIX
#define UART_PREFIX     log
#include "uart.h"
#define log_hex(b)      do { char log_buf[5]; sprintf (log_buf, "<%02x>", b); log_uart_puts (log_buf); } while (0)
#else
#define log_init(b)
#define log_putc(c)
#define log_puts(s)
#define log_message(m)
#define log_flush()
#define log_hex(b)
#endif

static void
esp8266_gpio_init (void)
{
    ESP8266_RST_PERIPH_CLOCK_CMD (ESP8266_RST_PERIPH, ENABLE);      // enable clock for ESP8266 RST
    GPIO_SET_PIN_OUT_PP (ESP8266_RST_PORT, ESP8266_RST_PIN, GPIO_Speed_2MHz);

    ESP8266_RST_PERIPH_CLOCK_CMD (ESP8266_CH_PD_PERIPH, ENABLE);    // enable clock for ESP8266 CH_PD
    GPIO_SET_PIN_OUT_PP (ESP8266_CH_PD_PORT, ESP8266_CH_PD_PIN, GPIO_Speed_2MHz);

    ESP8266_RST_PERIPH_CLOCK_CMD (ESP8266_FLASH_PERIPH, ENABLE);    // enable clock for ESP8266 FLASH
    GPIO_SET_PIN_OUT_PP (ESP8266_FLASH_PORT, ESP8266_FLASH_PIN, GPIO_Speed_2MHz);

    GPIO_SET_BIT(ESP8266_RST_PORT,   ESP8266_RST_PIN);
    GPIO_SET_BIT(ESP8266_CH_PD_PORT, ESP8266_CH_PD_PIN);
    GPIO_SET_BIT(ESP8266_FLASH_PORT, ESP8266_FLASH_PIN);
}
/*--------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: poll ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
esp8266_poll (uint_fast8_t * chp, uint_fast16_t ten_ms)
{
    uint_fast16_t  cnt = 0;

    esp8266_ten_ms_tick = 0;

    while (1)
    {
        if (esp8266_uart_poll (chp))
        {
            return 1;
        }

        if (esp8266_ten_ms_tick)
        {
            esp8266_ten_ms_tick = 0;

            cnt++;

            if (cnt >= ten_ms)
            {
                break;
            }
        }
    }
    return 0;
}

/* Die Faehigkeitsmeldung "CAP var-crc" (A32, Design 6.4) und ihre Festschreibung.
 *
 * Der ESP meldet die Marke einmal je Sitzung beim Hochfahren, VOR jeder WLAN-Meldung. Gueltig
 * wird sie erst mit der FIRMWARE-Zeile DERSELBEN Sitzung -- und genau das ist der Rueckfall gegen
 * die Rueckroll-Falle: Die FIRMWARE-Zeile sendet jeder ESP bei jedem Start (ESP-uclock.ino,
 * setup()), auch ein zurueckgerollter ohne Pruefsummenverstaendnis. Sie markiert damit den Beginn
 * einer neuen Sitzung, und ohne vorangegangenes CAP loescht sie das Flag.
 *
 * Warum das noetig ist: Ohne die Festschreibung ueberlebt ein einmal gesetztes Flag das
 * OTA-Rueckrollen des ESP, weil es nur ein STM-Neustart loescht. Der STM haengte dann die Marke an
 * eine Gegenstelle, die sie als Teil des Werts speichert -- "meinhost*c328" als Hostname, also
 * genau der stille falsche Wert, gegen den die Pruefsumme antritt (Risiko R-3).
 *
 * Die sichere Richtung ist dabei immer "keine Pruefsumme": Geht CAP verloren, verhaelt sich alles
 * wie vorher. Geht die FIRMWARE-Zeile einer RUECKROLL-Sitzung verloren, bleibt das Flag stehen --
 * das ist die verbleibende Luecke, und sie braucht beides: einen Verlust UND ein Rueckrollen.
 */
static uint_fast8_t     esp8266_cap_var_crc_seen = 0;           // CAP in dieser Sitzung gesehen, noch nicht festgeschrieben

static void
esp8266_cap_latch (void)
{
    esp8266.cap_var_crc      = esp8266_cap_var_crc_seen;
    esp8266_cap_var_crc_seen = 0;
}

/* Ein Zeichen gegen ein Suchwort pruefen, ohne Zeilenpuffer. Rueckgabe 1, sobald das Wort
 * vollstaendig gelesen wurde. Gebraucht wird das in esp8266_reset(), wo die Bootmeldungen
 * verworfen werden, bevor irgendein Zeilenparser laeuft.
 */
static uint_fast8_t
esp8266_scan_token (const char * token, uint_fast8_t * posp, uint_fast8_t ch)
{
    uint_fast8_t    pos = *posp;

    if ((uint_fast8_t) token[pos] == ch)
    {
        pos++;

        if (! token[pos])
        {
            *posp = 0;
            return 1;
        }
    }
    else
    {
        pos = ((uint_fast8_t) token[0] == ch) ? 1 : 0;
    }

    *posp = pos;
    return 0;
}

/* Zuordnung der Quittung (A35 Teil 1, S.16; specs/paket-2026-10-05/design.md 6.4).
 *
 * Der ESP sendet vor JEDER Quittung einer var-Zeile, dem Punkt wie dem "!v", eine eigene Zeile
 * "ACK xy": xy ist das hoehere Byte der Pruefsummenmarke, die er mit der Zeile EMPFANGEN hat, klein
 * geschrieben -- also das Byte, das var_send_buf() selbst gesendet hat; der ESP rechnet nichts nach.
 * "ACK --" heisst: keine Zuordnung, werten wie bisher -- NICHT "passt nicht".
 *
 * Bisher trug die Quittung keine Zuordnung: Eine verspaetete Quittung bestaetigte das NAECHSTE
 * Kommando, eine verspaetete Abweisung liess ein fremdes nachsenden (L233). Jetzt gilt:
 *
 *   - Die Zuordnung gilt NUR fuer die unmittelbar folgende Zeile. Jede vollstaendige Zeile
 *     verbraucht sie; folgt keine Quittung, ist sie weg und geht nicht auf ein spaeteres
 *     Kommando ueber (AKS.5).
 *   - Passt sie nicht zum Kommando, auf das var_send_buf() gerade wartet, wird der PUNKT
 *     VERWORFEN (ESP8266_ACK_DROP) und gezaehlt (diag a=). Das Kommando laeuft in seinen Timeout
 *     und wird von A32 nachgesendet -- das verspaetete genau einmal, das wartende gar nicht.
 *   - Ein "!v" geht dagegen IMMER als ESP8266_NAK durch, auch mit fremder Zuordnung (Review S.21,
 *     H1). Beim "!v" wegen falscher Pruefsumme sendet der ESP die EMPFANGENE Marke zurueck; war
 *     genau deren hohes Byte verfaelscht, machte die Zuordnung aus dem NAK einen DROP -- 3 s ohne
 *     watchdog_reload() und eine spaete statt einer sofortigen Nachsendung. Ein fremdes "!v" kostet
 *     schlimmstenfalls eine ueberfluessige, gleichwertige Nachsendung des wartenden Kommandos.
 *   - Keine Zuordnung ("--", keine ACK-Zeile, verstuemmelte Zeile), oder niemand wartet mit Marke
 *     (esp8266_ack_expect == ESP8266_ACK_NONE): wie bisher. Beide Regeln folgen demselben Grundsatz:
 *     Im Zweifel gilt der Stand vor S, nicht ein 3-s-Timeout ohne watchdog_reload().
 *
 * Eigener Zweig in der Praefixkette, VOR dem Zweig, der unbekannte Zeilen loggt: Sonst kostete jede
 * ACK-Zeile einen log_flush()-Busy-Wait (L266), rund 194 je Vollabgleich.
 */
uint_fast16_t           esp8266_ack_expect = ESP8266_ACK_NONE;          // gesetzt von var_send_buf(), waehrend es wartet
uint16_t                esp8266_ack_drops  = 0;                         // verworfene Quittungen, saettigend
static uint_fast16_t    esp8266_ack_tag    = ESP8266_ACK_NONE;          // aus der vorigen Zeile, gilt nur fuer die naechste

static uint_fast16_t
esp8266_ack_parse (const char * p)                                      // genau zwei Hexziffern, klein, wie der ESP sie sendet
{
    uint_fast16_t   val = 0;
    uint_fast8_t    i;
    uint_fast8_t    ch;

    for (i = 0; i < 2; i++)                                             // liest p[1] nur, wenn p[0] eine Hexziffer war
    {
        ch = (uint_fast8_t) p[i];

        if (ch >= '0' && ch <= '9')
        {
            ch -= '0';
        }
        else if (ch >= 'a' && ch <= 'f')
        {
            ch -= 'a' - 10;
        }
        else
        {
            return ESP8266_ACK_NONE;                                    // "--" und alles Verstuemmelte
        }

        val = (val << 4) | ch;
    }

    return p[2] ? ESP8266_ACK_NONE : val;
}

static uint_fast8_t
esp8266_ack_check (uint_fast8_t rtc, uint_fast16_t tag)
{
    if (tag != ESP8266_ACK_NONE && esp8266_ack_expect != ESP8266_ACK_NONE && tag != esp8266_ack_expect)
    {
        if (esp8266_ack_drops < 0xFFFF)                                 // saettigend
        {
            esp8266_ack_drops++;
        }

        return ESP8266_ACK_DROP;
    }

    return rtc;
}

/* strncpy MIT Abschlussbyte, fuer jedes Feld der Union esp8266.u (A13 / L96, A47 / L292).
 *
 * strncpy schreibt bei voller Laenge KEIN Nullbyte. Alle Felder liegen in derselben Union; ohne
 * Abschluss stuende hinter einem vollen Feld noch der Rest des vorigen, laengeren Inhalts, und jede
 * Stringfunktion laese ueber das Feld hinaus -- bei u.cmd war das Befund L90. Jedes Feld ist
 * [len + 1] gross, dst[len] liegt also immer im Feld.
 *
 * Eine Funktion statt je einer Abschlusszeile an 16 Stellen: Auf dem F103 ist das rund 90 Byte
 * billiger als acht weitere Zeilen, und es kann keine Stelle mehr den Abschluss vergessen.
 */
static void
esp8266_copy (char * dst, const char * src, uint_fast8_t len)
{
    strncpy (dst, src, len);
    dst[len] = 0;
}

/* Markierte Kommandos "CMC <nutzlast>*hhhh" (Teil D, specs/paket-2026-10-09, design.md 5.3/5.4).
 *
 * Der ESP markiert seine Kommandos, sobald die Eroeffnung des Vollabgleichs das Flag 0x04 traegt
 * (vars.c, var_send_all_variables()). hhhh ist var_crc() ueber die Nutzlast OHNE "CMC " und OHNE
 * die Marke -- DIESELBE Funktion wie fuer die var-Zeilen, keine zweite Rechnung.
 *
 * "CMD" ist nie markiert, "CMC" immer. Eine CMC-Zeile ohne gueltige Marke ist deshalb IMMER ein
 * Uebertragungsfehler: verkuerzt, mit einer anderen verschmolzen (L141, L188, L205) -- und wird
 * NICHT angewandt. Ein still beschaedigter Wert faellt niemandem auf, ein ausbleibender schon.
 *
 * Geprueft wird: '*' an fuenftletzter Stelle, davor mindestens ein Zeichen Nutzlast, die Nutzlast
 * passt in u.cmd, und die vier Zeichen danach sind BYTE-GLEICH mit "%04x" von var_crc() -- also
 * genau das, was der ESP sendet (stm_cmd_send()). Strenger als jede Ziffernauswertung und
 * ausdruecklich NICHT htoi(), das jedes Nicht-Hexzeichen still als 0 nimmt (L232): Drei Ziffern,
 * Nicht-Hex, Grossbuchstaben, ein fuenftes Zeichen -- alles ungleich, alles abgewiesen. Der
 * Vergleich statt einer Ziffernschleife spart auf dem F103 rund 47 Byte Flash (Gate, design.md 5.7).
 * Eine zu lange Nutzlast wird abgewiesen statt wie bei "CMD" gekuerzt angewandt -- eine gekuerzte
 * Zeile mit stimmender Marke waere genau der Widerspruch, gegen den die Marke antritt.
 *
 * Bei einer Abweisung (Ent-5 = N1):
 *   - Zaehler saettigend, Zeile "cmd abgewiesen #n len=l" ueber log_printf(): die ersten vier
 *     einzeln, danach jede fuenfzigste (L109: die Meldung liegt auf derselben Leitung). l ist die
 *     Laenge hinter "CMC ", samt Marke. KEIN Wert -- Zeichenketten tragen Hosts und Zugangsdaten.
 *   - Vollabgleich vormerken, hoechstens einer je 60 s: Der ESP hat den neuen Wert bei sich schon
 *     gesetzt; der Abgleich zieht ihn auf den Stand des STM zurueck, statt dass beide bis zum
 *     naechsten Abgleich auseinanderlaufen (L42). Gedrosselt, weil ein Abgleich rund 200 quittierte
 *     Zeilen sind und eine Abweisung meist von einer ueberlasteten Bruecke kommt. Gesendet wird
 *     im Hauptloop (main.c, var_sync_pending) -- nie hier, nie verschachtelt.
 */
uint_fast8_t            esp8266_cmc_sync      = 0;                      // abgewiesen: Vollabgleich faellig, main.c loescht
static uint16_t         esp8266_cmc_rejects   = 0;                      // abgewiesene CMC-Zeilen, saettigend
static uint32_t         esp8266_cmc_sync_next = 0;                      // uptime, ab der wieder vorgemerkt werden darf

static uint_fast8_t
esp8266_cmc_check (char * p)                                            // p hinter "CMC "; 1 = Marke stimmt und ist abgeschnitten
{
    uint_fast16_t   len = (uint_fast16_t) strlen (p);                   // answer ist terminiert und hoechstens 256 Zeichen lang

    if (len >= 6 && len <= ESP8266_MAX_CMD_LEN + 5 && p[len - 5] == '*')
    {
        char    mark[5];                                                // "%04x" + Nullbyte

        p[len - 5] = '\0';                                              // Marke ab, gerechnet wird ueber die Nutzlast
        sprintf (mark, "%04x", (unsigned int) var_crc (p));

        if (! strcmp (mark, p + len - 4))
        {
            return 1;
        }
    }

    if (esp8266_cmc_rejects < 0xFFFF)
    {
        esp8266_cmc_rejects++;
    }

    if (esp8266_cmc_rejects <= 4 || (esp8266_cmc_rejects % 50) == 0)
    {
        log_printf ("cmd abgewiesen #%u len=%u\r\n", (unsigned int) esp8266_cmc_rejects, (unsigned int) len);
    }

    if (uptime >= esp8266_cmc_sync_next)
    {
        esp8266_cmc_sync_next = uptime + 60;
        esp8266_cmc_sync      = 1;
    }

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * get message from ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
esp8266_get_message (void)
{
    static char         answer[ESP8266_MAX_ANSWER_LEN + 1];
    static uint_fast8_t answer_pos = 0;
    uint_fast8_t        ch;
    uint_fast8_t        rtc = ESP8266_TIMEOUT;

    log_flush ();

    /* Hier stand esp8266_uart_flush (). Die ABHOLfunktion wartete als Erstes darauf, dass der
     * SENDEpuffer leer ist -- und blockierte damit genau das Abholen, dessen Ausbleiben den
     * Empfangsring ueberlaufen laesst. Je mehr der STM loggt, desto spaeter liest er; das ist
     * eine Mitkopplung, keine Absicherung (BEFUNDE.md, L144).
     *
     * Das Senden braucht den Flush hier nicht: esp8266_send_cmd(), esp8266_send_data() und
     * var_send_buf() flushen jeweils selbst, nachdem sie ihr Kommando abgelegt haben.
     */

    if (esp8266_uart_char_available ())
    {
        while (1)
        {
            if (esp8266_uart_poll (&ch) == 0)                                 // don't wait
            {
                rtc = ESP8266_TIMEOUT;
                break;
            }

            esp8266.is_up = 1;

            if (ch == '\n')
            {
                answer[answer_pos] = '\0';

                if (answer_pos > 0)
                {
                    uint_fast16_t   ack_tag = esp8266_ack_tag;                              // Zuordnung aus der VORIGEN Zeile ...

                    esp8266_ack_tag = ESP8266_ACK_NONE;                                     // ... gilt nur fuer diese, was immer sie ist
                    answer_pos = 0;

                    if (answer[0] == '.' && answer[1] == '\0')                              // dot as "silent ok"
                    {
                        rtc = esp8266_ack_check (ESP8266_OK, ack_tag);
                        break;
                    }
                    else if (answer[0] == '!' && answer[1] == 'v' && answer[2] == '\0')     // "!v": Zeile abgelehnt
                    {
                        /* Der ESP hat die Zeile gelesen, ihre Pruefsumme stimmte aber nicht -- er hat sie
                         * deshalb NICHT angewandt (A32, Design 6.2). Das ist ein Lebenszeichen der Bruecke
                         * und kein Erfolg; var_send_buf() unterscheidet beides und merkt das Kommando sofort
                         * zur Nachsendung vor. Still wie der Punkt: Gemeldet hat es bereits der ESP selbst,
                         * und jede zusaetzliche Zeile laege auf derselben ueberlasteten Leitung (L109).
                         */
                        rtc = ESP8266_NAK;                                                  // ohne Zuordnungspruefung, H1: siehe esp8266_ack_check()
                        break;
                    }
                    else if (! strncmp (answer, "ACK ", 4))                                 // Zuordnung der folgenden Quittung, still
                    {
                        esp8266_ack_tag = esp8266_ack_parse (answer + 4);
                        rtc = ESP8266_ACK;
                        break;
                    }
                    else if (! strncmp (answer, "FILE ", 5))                                // FILE: keep silent
                    {
                        esp8266_copy (esp8266.u.filedata, answer + 5, ESP8266_MAX_CMD_LEN);
                        rtc = ESP8266_FILEDATA;
                        break;
                    }
                    else if (! strncmp (answer, "ICON ", 5))                                // ICON: keep silent
                    {
                        esp8266_copy (esp8266.u.filedata, answer + 5, ESP8266_MAX_CMD_LEN);
                        rtc = ESP8266_ICONDATA;
                        break;
                    }
                    else if (! strncmp (answer, "TAB", 3))                                  // TABxxx: keep silent
                    {
                        if (! strcmp (answer, "TABLES"))                                    // TABLES
                        {
                            rtc = ESP8266_TABLES;
                            break;
                        }
                        else if (! strncmp (answer + 3, "INFO ", 5))                        // TABINFO
                        {
                            esp8266_copy (esp8266.u.tabinfo, answer + 8, ESP8266_TABINFO_LEN);
                            rtc = ESP8266_TABINFO;
                            break;
                        }
                        else if (! strncmp (answer + 3, "ILLU ", 5))                        // TABILLU
                        {
                            esp8266_copy (esp8266.u.tabillu, answer + 8, ESP8266_TABILLU_LEN);
                            rtc = ESP8266_TABILLU;
                            break;
                        }
#if WCLOCK24H == 1
                        else if (! strncmp (answer + 3, "T ", 2))                           // TABT
                        {
                            esp8266_copy (esp8266.u.tabt, answer + 5, ESP8266_TABT_LEN);
                            rtc = ESP8266_TABT;
                            break;
                        }
#endif
                        else if (! strncmp (answer + 3, "H ", 2))                           // TABH
                        {
                            esp8266_copy (esp8266.u.tabh, answer + 5, ESP8266_TABH_LEN);
                            rtc = ESP8266_TABH;
                            break;
                        }
                        else if (! strncmp (answer + 3, "M ", 2))                           // TABM
                        {
                            esp8266_copy (esp8266.u.tabm, answer + 5, ESP8266_TABM_LEN);
                            rtc = ESP8266_TABM;
                            break;
                        }
                        else
                        {
                            rtc = ESP8266_UNSPECIFIED;
                            break;
                        }
                    }
                    else
                    {
                        log_puts ("(");
                        log_puts (answer);
                        log_puts (")\r\n");
                        log_flush ();

                        if (! strcmp (answer, "CAP var-crc"))
                        {
                            /* Nur gemerkt, nicht sofort gueltig: Festgeschrieben wird die Faehigkeit mit
                             * der FIRMWARE-Zeile derselben Sitzung (esp8266_cap_latch(), Begruendung dort).
                             * Exakter Vergleich ohne Parameter -- der ESP ist fuer den STM keine
                             * vertrauenswuerdige Quelle.
                             */
                            esp8266_cap_var_crc_seen = 1;
                            rtc = ESP8266_STATUS;                                       // Statusmeldung, KEINE Quittung
                            break;
                        }
                        else if (! strncmp (answer, "OK", 2))
                        {
                            /* Die Quittung eines var-Kommandos ist der Punkt (oben, :227) -- der ESP
                             * sendet ihn in ESP-uclock.ino:444 unmittelbar nach var_set_parameter().
                             * Eine Zeile mit "OK" ist NIE eine Quittung. Der ESP sendet genau drei
                             * davon, alle unaufgefordert waehrend seines Bootlaufs:
                             *
                             *   "OK cap"   wifi.cpp:68    statusmsg beim Hochfahren
                             *   "OK ap"    wifi.cpp:144   Wechsel in den AP-Modus
                             *   "OK time"  ntp.cpp:101    nach dem NTP-Abgleich
                             *
                             * statusmsg() (base.cpp:264) kennt keinen weiteren OK-Aufruf: abgezaehlt,
                             * nicht vermutet. Als ESP8266_OK quittierte jede dieser drei Zeilen ein
                             * fremdes Kommando, verschob die Paarung dauerhaft um eins und lud seit
                             * 3.2.14 zusaetzlich den Watchdog in var_send_buf() auf eine Falschmeldung
                             * hin nach. Deshalb ein eigener Rueckgabewert (specs/bruecke, Design 1).
                             */
                            rtc = ESP8266_STATUS;
                            break;
                        }
                        else if (! strncmp (answer, "ERROR", 5))
                        {
                            rtc = ESP8266_ERROR;
                            break;
                        }
                        else if (! strncmp (answer, "- ", 2))
                        {
                            rtc = ESP8266_DEBUGMSG;
                            break;
                        }
                        else if (! strncmp (answer, "DISP ", 5))
                        {
                            esp8266_copy (esp8266.u.disp, answer + 5, ESP8266_DISP_LEN);
                            rtc = ESP8266_DISP;
                            break;
                        }
                        else if (! strncmp (answer, "IPADDRESS ", 10))
                        {
                            strncpy (esp8266.ipaddress, answer + 10, ESP8266_MAX_IPADDRESS_LEN);
                            esp8266.is_online = 1;
                            rtc = ESP8266_IPADDRESS;
                            break;
                        }
                        else if (! strcmp (answer, "SYNCVARS"))
                        {
                            /* Der ESP bittet um den vollen Variablensatz (A6, Weg B; BEFUNDE.md L231).
                             * Er sendet das nur, wenn er einige Sekunden nach seinem Hochlauf findet,
                             * dass HARDWARE_CONFIGURATION noch auf dem Vorgabewert 65535 steht -- also
                             * genau dann, wenn der Satz nicht angekommen ist (L42, L103).
                             *
                             * Exakter Vergleich OHNE Parameter, aus demselben Grund wie bei "CAP var-crc"
                             * weiter oben: Der ESP ist fuer den STM keine vertrauenswuerdige Quelle.
                             * "SYNCVARS 1" und "SYNCVARSX" sind damit keine Anforderung, sondern
                             * ESP8266_UNSPECIFIED.
                             *
                             * is_online wird hier MITgesetzt, und das ist der eigentliche Grund fuer
                             * diesen Zweig. Bisher gab es dafuer genau eine Stelle -- die Zeile
                             * darueber. An dem Flag haengen 15 Sendepfade in main.c; geht die
                             * IPADDRESS-Zeile verloren, haelt der STM den ESP fuer offline und
                             * schweigt, und nichts ausser einem Neustart heilt das (L255).
                             *
                             * esp8266.ipaddress bleibt dagegen unberuehrt: Die Adresse steht nur in der
                             * IPADDRESS-Zeile, und der ESP sendet sie beim naechsten WLAN-Ereignis
                             * ohnehin nach. Ein erfundener Wert waere schlimmer als ein leeres Feld.
                             *
                             * Eigener Rueckgabewert statt ESP8266_IPADDRESS, weil dessen Zweig in main.c
                             * den IP-Lauftext ueber die Uhr schickt -- genau die sichtbare Nebenwirkung,
                             * derentwegen der Nutzer Weg B gewaehlt hat (L231).
                             */
                            esp8266.is_online = 1;
                            rtc = ESP8266_SYNCVARS;
                            break;
                        }
                        else if (! strncmp (answer, "AP ", 3))
                        {
                            strncpy (esp8266.accesspoint, answer + 3, ESP8266_MAX_ACCESSPOINT_LEN);
                            rtc = ESP8266_ACCESSPOINT;
                            break;
                        }
                        else if (! strncmp (answer, "MODE ", 5))
                        {
                            if (! strncmp (answer + 5, "ap", 2))
                            {
                                esp8266.mode = ESP8266_AP_MODE;
                            }
                            else
                            {
                                esp8266.mode = ESP8266_CLIENT_MODE;
                            }
                            rtc = ESP8266_MODE;
                            break;
                        }
                        else if (! strncmp (answer, "TIME ", 5))
                        {
                            esp8266_copy (esp8266.u.time, answer + 5, ESP8266_MAX_TIME_LEN);
                            rtc = ESP8266_TIME;
                            break;
                        }
                        else if (! strncmp (answer, "WEATHER ", 8))
                        {
                            esp8266_copy (esp8266.u.weather, answer + 8, ESP8266_MAX_WEATHER_LEN);
                            rtc = ESP8266_WEATHER;
                            break;
                        }
                        else if (! strncmp (answer, "WEATHER_FC ", 11))
                        {
                            esp8266_copy (esp8266.u.weather, answer + 11, ESP8266_MAX_WEATHER_LEN);
                            rtc = ESP8266_WEATHER_FC;
                            break;
                        }
                        else if (! strncmp (answer, "WICON ", 6))
                        {
                            esp8266_copy (esp8266.u.weather, answer + 6, 2);                         // copy only 2 characters ("02d" -> "02")
                            rtc = ESP8266_WEATHER_ICON;
                            break;
                        }
                        else if (! strncmp (answer, "WICON_FC ", 9))
                        {
                            esp8266_copy (esp8266.u.weather, answer + 9, 2);                         // copy only 2 characters ("02d" -> "02")
                            rtc = ESP8266_WEATHER_FC_ICON;
                            break;
                        }
                        else if (! strncmp (answer, "FIRMWARE ", 9))
                        {
                            strncpy (esp8266.firmware, answer + 9, ESP8266_MAX_FIRMWARE_LEN);
                            esp8266_cap_latch ();                                       // Ende der Startmeldungen: Faehigkeit dieser Sitzung festschreiben
                            rtc = ESP8266_FIRMWARE;
                            break;
                        }
                        else if (! strncmp (answer, "CMD ", 4) || ! strncmp (answer, "CMC ", 4))
                        {
                            if (answer[2] == 'C' && ! esp8266_cmc_check (answer + 4))
                            {
                                rtc = ESP8266_UNSPECIFIED;                              // Teil D: abgewiesen, NICHT angewandt
                                break;
                            }

                            /* Ab hier "CMD" wie bisher, oder "CMC" mit stimmender, abgeschnittener Marke:
                             * DERSELBE Weg (design.md 5.4).
                             */
                            esp8266_copy (esp8266.u.cmd, answer + 4, ESP8266_MAX_CMD_LEN);
                            /* Befund L90: strncpy schreibt bei voller Laenge KEIN Nullbyte. u.cmd liegt in einer
                             * union mit u.filedata; ohne Abschluss steht an Position 127 noch ein Byte des
                             * vorangegangenen Dateikommandos. Den Abschluss setzt esp8266_copy().
                             */
                            rtc = ESP8266_CMD;
                            break;
                        }
                        else if (! strncmp (answer, "OPEN ", 5))
                        {
                            esp8266_copy (esp8266.u.filedata, answer + 5, ESP8266_MAX_CMD_LEN);
                            rtc = ESP8266_FILEOPEN;
                            break;
                        }
                        else if (! strncmp (answer, "FILE ", 5))
                        {
                            esp8266_copy (esp8266.u.filedata, answer + 5, ESP8266_MAX_CMD_LEN);
                            rtc = ESP8266_FILEDATA;
                            break;
                        }
                        else if (! strcmp (answer, "CLOSE"))
                        {
                            rtc = ESP8266_FILECLOSE;
                            break;
                        }
                        else
                        {
                            rtc = ESP8266_UNSPECIFIED;
                            break;
                        }
                    }
                }
                else // l == 0
                {
                    log_flush ();
                }
            }
            else if (ch == '\r')
            {
                ;
            }
            else if (answer_pos < ESP8266_MAX_ANSWER_LEN)
            {
                answer[answer_pos++] = ch;
            }
        }
    }

    if (rtc != ESP8266_TIMEOUT)
    {
        answer[0] = '\0';
    }
    return rtc;
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send a command to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
void
esp8266_send_cmd (const char * cmd, const char * args, uint_fast8_t do_log)
{
    uint_fast8_t    length;
    uint_fast8_t    ch;
    uint_fast8_t    i;

    if (do_log)
    {
        log_puts ("--> ");

        length = strlen (cmd);

        for (i = 0; i < length; i++)
        {
            ch = cmd[i];

            if (ch >= 32 && ch < 127)
            {
                log_putc (ch);
            }
            else
            {
                log_hex (ch);
            }
        }

        if (args)
        {
            log_putc (' ');
            log_putc ('"');

            length = strlen (args);

            for (i = 0; i < length; i++)
            {
                ch = args[i];

                if (ch >= 32 && ch < 127)
                {
                    log_putc (ch);
                }
                else
                {
                    log_hex (ch);
                }
            }
            log_putc ('"');
        }

        log_puts ("<0d><0a>\r\n");
        log_flush ();
    }

    esp8266_uart_puts (cmd);

    if (args)
    {
        esp8266_uart_putc (' ');
        esp8266_uart_putc ('"');
        esp8266_uart_puts (args);
        esp8266_uart_putc ('"');
    }

    esp8266_uart_puts ("\r\n");
    esp8266_uart_flush();
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send (http) data to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
void
esp8266_send_data (unsigned char * data, uint_fast8_t len)
{
    uint_fast8_t    i;

    for (i = 0; i < len; i++)
    {
        esp8266_uart_putc (data[i]);
    }

    esp8266_uart_flush ();

    return;
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * send a debug log line to ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
void
esp8266_send_log_line (const char * line)
{
    char ch;

    if (! esp8266_uart_ready || ! line)
    {
        return;
    }

    while (*line == '\r' || *line == '\n')
    {
        line++;
    }

    if (! *line)
    {
        return;
    }

    esp8266_uart_puts ("LOG ");

    while ((ch = *line++) != '\0')
    {
        if (ch == '\r')
        {
            continue;
        }

        if (ch == '\n')
        {
            break;
        }

        esp8266_uart_putc (ch);
    }

    esp8266_uart_puts ("\r\n");

    /* Kein esp8266_uart_flush() mehr. Die Logspiegelung ist der haeufigste Sender auf dieser
     * Leitung, und der Flush wartete, bis das LETZTE Zeichen draussen war -- bei 115200 Baud
     * rund 0,09 ms je Zeichen, bei einer 70-Byte-Zeile also rund 6 ms Hauptloop-Stillstand fuer
     * eine Diagnoseausgabe. In dieser Zeit holt niemand vom ESP ab, und der Empfangsring
     * verwirft bei Ueberlauf still (BEFUNDE.md, L144).
     *
     * Nichts geht dadurch verloren: Die TXE-ISR leert den Ring selbstaendig, und uart_putc()
     * uebt bei vollem Ring von sich aus Gegendruck (uart-driver.h). Das Warten wird damit von
     * "immer" auf "nur wenn der Ring wirklich voll ist" zurueckgenommen.
     *
     * Zusatznutzen auf dem Fehlerpfad: fault_reset() (main.c) schaltet die Interrupts AB und
     * protokolliert danach ueber log_printf(). Der Flush hier haette dort auf eine ISR gewartet,
     * die nicht mehr laeuft.
     */
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * reset ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
void
esp8266_reset (void)
{
    static const char   cap_token[] = "CAP var-crc";
    static const char   fw_token[]  = "FIRMWARE ";
    uint_fast8_t        cap_pos = 0;
    uint_fast8_t        fw_pos = 0;
    uint_fast8_t        ch;

    esp8266.cap_var_crc      = 0;                                           // neue Sitzung: bis zur Meldung keine Pruefsumme anhaengen
    esp8266_cap_var_crc_seen = 0;

    GPIO_RESET_BIT(ESP8266_RST_PORT, ESP8266_RST_PIN);
    delay_msec (50);
    GPIO_SET_BIT(ESP8266_RST_PORT, ESP8266_RST_PIN);

    /* Die Bootmeldungen werden weiterhin verworfen -- aber nicht mehr blind. Diese Schleife
     * laeuft, solange hoechstens 500 ms Pause zwischen zwei Zeichen liegen, und sie ist der
     * einzige Leser, bevor der Hauptloop anlaeuft. Faellt die Faehigkeitsmeldung in dieses
     * Fenster, saehe sie sonst niemand, und die Pruefsumme bliebe die ganze Sitzung lang aus --
     * stillschweigend. Heute trennt ein delay(1000) im setup() des ESP beides, aber dieses
     * Fenster haengt an fremdem Code und verschiebt sich mit ihm.
     *
     * Gesucht wird ohne Zeilenpuffer, zeichenweise: Der ESP schickt hier auch Bootgeroell
     * seines ROM-Laders mit falscher Baudrate, und das ist keine Zeile.
     */
    while (esp8266_poll (&ch, MSEC(500)))                                   // eat boot message stuff
    {
        if (esp8266_scan_token (cap_token, &cap_pos, ch))                   // beide Suchworte, nicht else-if:
        {                                                                   // ein Zeichen darf zu beiden beitragen
            esp8266_cap_var_crc_seen = 1;
        }

        if (esp8266_scan_token (fw_token, &fw_pos, ch))
        {
            esp8266_cap_latch ();                                           // dieselbe Regel wie im Zeilenparser
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * power down ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
void
esp8266_powerdown (void)
{
    GPIO_RESET_BIT(ESP8266_CH_PD_PORT, ESP8266_CH_PD_PIN);
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * power up ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
void
esp8266_powerup (void)
{
    GPIO_SET_BIT(ESP8266_CH_PD_PORT, ESP8266_CH_PD_PIN);
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * connect to access point
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
void
esp8266_connect_to_access_point (char * ssid, char * key)
{
    char send_cmd_buf[ESP8266_MAX_SEND_CMD_LEN + 1];

    esp8266.ipaddress[0] = '\0';
    esp8266.is_online = 0;

    sprintf (send_cmd_buf, "cap \"%s\",\"%s\"", ssid, key);
    esp8266_send_cmd (send_cmd_buf, (const char *) NULL, 1);
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * start as access point
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
void
esp8266_accesspoint (const char * ssid, const char * key)
{
    char send_cmd_buf[ESP8266_MAX_SEND_CMD_LEN + 1];

    esp8266.ipaddress[0] = '\0';
    esp8266.is_online = 0;

    sprintf (send_cmd_buf, "ap \"%s\",\"%s\"", ssid, key);
    esp8266_send_cmd (send_cmd_buf, (const char *) NULL, 1);
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * start WPS
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
void
esp8266_wps (void)
{
    esp8266.ipaddress[0] = '\0';
    esp8266.is_online = 0;

    esp8266_send_cmd ("wps", (const char *) NULL, 1);
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * set ESP8266 into flash mode and copy all characters from extern logger UART to ESP8266 UART
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
void
esp8266_flash (void)
{
    uint_fast8_t    ch;

    /* Der einzige Flush, der auf diesem Pfad bleiben muss. Was hier noch im TX-Ring steht,
     * gehoert zur bisherigen Kommandobruecke; gleich darunter wird der UART neu aufgesetzt und
     * danach ist diese Leitung eine reine Durchreiche zum Flashwerkzeug. Was jetzt nicht draussen
     * ist, mischt sich sonst in den Flashverkehr.
     */
    esp8266_uart_flush ();

    log_init (115200);
    esp8266_gpio_init ();
    esp8266_uart_init (115200);
    esp8266_uart_ready = 1;

    GPIO_RESET_BIT(ESP8266_FLASH_PORT, ESP8266_FLASH_PIN);
    GPIO_RESET_BIT(ESP8266_RST_PORT, ESP8266_RST_PIN);
    delay_msec (50);                                                                        // wait 50 msec
    GPIO_SET_BIT(ESP8266_RST_PORT, ESP8266_RST_PIN);
    delay_msec (500);                                                                       // wait 500 msec
    GPIO_SET_BIT(ESP8266_FLASH_PORT, ESP8266_FLASH_PIN);

    while (1)
    {
        if (esp8266_uart_poll (&ch))
        {
            log_uart_putc (ch);
        }

        if (log_uart_poll (&ch))
        {
            esp8266_uart_putc (ch);
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------
 * initialize UART and ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
void
esp8266_init (void)
{
    static uint_fast8_t     already_called;

    if (! already_called)
    {
        already_called = 1;

        log_init (115200);
        log_puts ("ESP8266 LOGGER\r\n");

        esp8266_gpio_init ();
        esp8266_uart_init (115200);
        esp8266_uart_ready = 1;
        esp8266_reset ();
    }

    return;
}
