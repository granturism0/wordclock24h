/*----------------------------------------------------------------------------------------------------------------------------------------
 * ESP-uclock.ino - some ESP8266 network routines with communication interface via UART to WordClock (STM32)
 *
 * Copyright (c) 2016-2025 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Arduino Settings ("Werkzeuge"):
 *     Board:                   Generic ESP8266 Module
 *     Upload Speed:            115200
 *     CPU-Frequency:           80 MHz
 *     Crystal Frqquency:       26 MHz
 *     Flash Frequency:         40 MHz
 *     Flash Mode:              DOUT
 *
 * Commands:
 *    cap apname,key            - connect to AP
 *    ap apname,key             - start local AP
 *    time [timeserver]         - get time from timeserver
 *    weather appid,city        - get weather of city
 *    weather appid,lon,lat     - get weather of location with coordinates (lon/lat)
 *
 * Return values:
 *    OK [string]               - Ok
 *    ERROR [string]            - Error
 * 
 * Examples:
 *    cap "fm7320","4711471147114711"
 *    ap "wordclock","1234567890"
 *    time "129.6.15.28"
 *    weather "123456789012345678901234567890","koeln"
 *    weather "123456789012345678901234567890","6.957","50.937"
 *    var "..."
 * 
 * Asynchronous Messages terminated with CR LF:
 *    - string                - Debug message, should be ignored
 *    FIRMWARE x.x.x          - Firmware version
 *    AP ssid                 - SSID of AP connected or own AP ssid
 *    MODE client             - working as WLAN client
 *    MODE ap                 - working as AP
 *    IPADDRESS x.x.x.x       - IP address of module is x.x.x.x
 *    TIME sec                - Time in seconds since 1900
 *    CMD xx ...              - Got command followed by parameters (printed in hex)
 *    HTTP GET path [param]   - HTTP request with path and optional params. Waits for answer lines until single dot (".") arrives.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiUdp.h>
#include <LittleFS.h>
#include <EEPROM.h>
#include "FS.h"
#include "base.h"
#include "stm32flash.h"
#include "wifi.h"
#include "http.h"
#include "udpsrv.h"
#include "ntp.h"
#include "weather.h"
#include "vars.h"
#include "display.h"
#include "tables.h"
#include "eepromdata.h"
#include "version.h"


#define CMD_BUFFER_SIZE     128                                             // maximum size of command buffer
/* 32 statt 64 Zeilen: spart 3'872 Byte im statischen Bereich (BEFUNDE.md L175/L176).
 * Der freie Haufen ist nicht das Problem, der GROESSTE ZUSAMMENHAENGENDE BLOCK ist es -
 * er faellt im Betrieb um rund 44 Prozent. Was hier an BSS frei wird, steht dem Haufen
 * dauerhaft zur Verfuegung. Die ZEILENLAENGE bleibt bei 120: unsere Diagnosezeile ist
 * 118 Zeichen lang, dort waere nichts zu holen, ohne das aussagekraeftigste Protokoll
 * zu beschneiden. Die 32 sind ausdruecklich gewaehlt und nicht 24 - die Rueckschau
 * wird gerade gebraucht.
 */
#define STM32_LOG_LINES     32
#define STM32_LOG_LINE_LEN  120
#define LOG_TRUNC_MARK      '~'                                             // Zeilenende-Marke: Text wurde gekuerzt

static void           icon_info (const char * fname, const char * name);
static const char *   resolve_icon_asset_filename (const char * fname);

static char              stm32_log_lines[STM32_LOG_LINES][STM32_LOG_LINE_LEN + 1];
static uint16_t          stm32_log_next_idx = 0;
static uint16_t          stm32_log_line_count = 0;

void
stm32_log_append (const char * line)
{
    if (! line || ! *line)
    {
        return;
    }

    strncpy (stm32_log_lines[stm32_log_next_idx], line, STM32_LOG_LINE_LEN);
    stm32_log_lines[stm32_log_next_idx][STM32_LOG_LINE_LEN] = '\0';

    if (strlen (line) > STM32_LOG_LINE_LEN)                                 // gekuerzt? Marke statt letztem Zeichen,
    {                                                                       // sonst sieht die Zeile vollstaendig aus
        stm32_log_lines[stm32_log_next_idx][STM32_LOG_LINE_LEN - 1] = LOG_TRUNC_MARK;
    }

    stm32_log_next_idx = (stm32_log_next_idx + 1) % STM32_LOG_LINES;

    if (stm32_log_line_count < STM32_LOG_LINES)
    {
        stm32_log_line_count++;
    }
}

void
stm32_log_clear (void)
{
    uint16_t idx;

    for (idx = 0; idx < STM32_LOG_LINES; idx++)
    {
        stm32_log_lines[idx][0] = '\0';
    }

    stm32_log_next_idx = 0;
    stm32_log_line_count = 0;
}

uint16_t
stm32_log_get_count (void)
{
    return stm32_log_line_count;
}

const char *
stm32_log_get_line (uint16_t idx)
{
    uint16_t start_idx;
    uint16_t real_idx;

    if (idx >= stm32_log_line_count)
    {
        return "";
    }

    start_idx = (stm32_log_next_idx + STM32_LOG_LINES - stm32_log_line_count) % STM32_LOG_LINES;
    real_idx = (start_idx + idx) % STM32_LOG_LINES;

    return stm32_log_lines[real_idx];
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * global setup
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
setup() 
{
    Serial.begin(115200);
    delay(1);
    Serial.println ("");
    Serial.flush ();
    delay(1000);

    /* Faehigkeitsmeldung der Bruecken-Pruefsumme (A32, Design 6.4), EINMAL je Sitzung.
     *
     * SIE MUSS VOR JEDER WLAN-MELDUNG UND VOR DER FIRMWARE-ZEILE STEHEN, und das ist keine
     * Stilfrage: Der STM behandelt CAP als blossen Merker und schreibt die Faehigkeit erst mit
     * der FIRMWARE-Zeile DERSELBEN Sitzung fest (src/esp8266/esp8266.c, esp8266_cap_latch()).
     * Kaeme CAP danach, bliebe das Flag geloescht, der STM haengte nie eine Pruefsumme an --
     * stillschweigend und ohne jede Meldung.
     *
     * Erst NACH dem delay(1000) darueber: Davor laeuft das Bootgeroell des ROM-Laders mit
     * falscher Baudrate ueber dieselbe Leitung. Der STM sucht die Marke zwar auch im
     * Verwurffenster von esp8266_reset() zeichenweise, aber darauf baut hier nichts.
     *
     * Restrisiko, benannt (Design 6.4): Geht die Zeile im 256-Byte-Ring des STM verloren, bleibt
     * die Pruefung bis zum naechsten ESP-Neustart aus. Das ist die sichere Richtung -- ohne
     * Pruefsumme verhaelt sich alles wie vorher, mit falsch angehaengter wuerden Werte beschaedigt.
     *
     * Kosten: 13 Byte, einmal je ESP-Start.
     */
    Serial.println ("CAP var-crc");
    Serial.flush ();

#if 0
    Serial.println ("- formatting filesystem");
    Serial.flush ();
    LittleFS.begin ();
    LittleFS.format ();
    LittleFS.end ();
    Serial.println ("- formatting ready");
    Serial.flush ();
#endif

    EEPROM.begin(512);                                        // use 512 bytes as EEPROM
    delay(10);
    eeprom_read ();

    if (eeprom_flags & EEPROM_FLAG_BOOT_AS_AP)
    {
        wifi_ap (eeprom_ap_ssid, eeprom_ap_ssidkey);
    }
    else
    {
        wifi_connect (eeprom_ssid, eeprom_ssidkey, false);
    }

    ntp_setup ();
    udp_server_setup ();
    Serial.print ("FIRMWARE ");
    Serial.println (ESP_VERSION);
    Serial.flush ();
    vars_init ();
    stm32_flash_init ();
    display_layout_init ();
    tables_init ();                                                // must be the last function, because it sends an info to STM32
}

#define MAX_ICON_SIZE   (32*32)

static char           icon_name[32 + 1];
static char           icon_colors[MAX_ICON_SIZE + 1];
static uint_fast16_t  icon_colors_len;
static char           icon_animations_on[MAX_ICON_SIZE + 1];
static char           icon_animations_off[MAX_ICON_SIZE + 1];
static uint_fast16_t  icon_anim_on_len;
static uint_fast16_t  icon_anim_off_len;
static uint_fast8_t   icon_rows;
static uint_fast8_t   icon_cols;
static uint_fast8_t   icon_found;

static uint_fast16_t  colors_pos;
static uint_fast16_t  anim_on_pos;
static uint_fast16_t  anim_off_pos;

static const char *
resolve_icon_asset_filename (const char * fname)
{
    static const char * const icon_candidates[] = { "wc24h-icon.txt", "wc12h-icon.txt", "uc-icon.txt" };
    static const char * const weather_candidates[] = { "wc24h-weather.txt", "wc12h-weather.txt", "uc-weather.txt" };
    const char * const * candidates = (const char * const *) 0;
    size_t candidate_count = 0;
    size_t idx;

    if (fname && LittleFS.exists (fname))
    {
        return fname;
    }

    if (fname && strstr (fname, "-weather.txt"))
    {
        candidates = weather_candidates;
        candidate_count = sizeof (weather_candidates) / sizeof (weather_candidates[0]);
    }
    else if (fname && strstr (fname, "-icon.txt"))
    {
        candidates = icon_candidates;
        candidate_count = sizeof (icon_candidates) / sizeof (icon_candidates[0]);
    }

    for (idx = 0; idx < candidate_count; idx++)
    {
        if (LittleFS.exists (candidates[idx]))
        {
            return candidates[idx];
        }
    }

    return fname;
}

static void
icon_info (const char * fname, const char * name)
{
    int     ch;
    int     i;
    int     len;
    File    fp;

    icon_rows = 0;
    icon_cols = 0;
    icon_colors_len = 0;
    icon_anim_on_len = 0;
    icon_anim_off_len = 0;
    icon_found = 0;

    LittleFS.begin ();
    fname = resolve_icon_asset_filename (fname);

    fp = LittleFS.open (fname, "r");

    if (fp)
    {
        ch = fp.read ();

        while (ch == '*')
        {
            i = 0;

            while ((ch = fp.read ()) != EOF)
            {
                if (ch == '\r' || ch == '\n')
                {
                    break;
                }

                if (i < 32)
                {
                    icon_name[i++] = ch;
                }
            }

            icon_name[i] = '\0';
            trim (icon_name);

            i                   = 0;
            icon_cols           = 0;
            icon_rows           = 0;
            icon_colors_len     = 0;
            icon_anim_on_len    = 0;
            icon_anim_off_len   = 0;
            len                 = 0;

            while ((ch = fp.read()) >= 0)
            {
                if (ch != '\r' && ch != '\n')
                {
                    if (ch == '*' || ch == '-')
                    {
                        break;
                    }

                    if (ch != ' ' && ch != '\t')
                    {
                        if (i < MAX_ICON_SIZE)
                        {
                          icon_colors[i++] = ch;
                        }
                        len++;
                    }
                }
                else
                {
                    if (icon_cols == 0)
                    {
                        if (i > 0)
                        {
                            icon_cols = i;
                        }
                    }
                }
            }

            icon_colors[i] = '\0';
            icon_colors_len = i;

            icon_rows = len / icon_cols;

            // animations on section
            if (ch == '-')
            {
                i = 0;

                while ((ch = fp.read ()) != EOF)
                {
                    if (ch != '\r' && ch != '\n')
                    {
                        if (ch == '*' || ch == '-')
                        {
                            break;
                        }

                        if (ch != ' ' && ch != '\t')
                        {
                            if (i < MAX_ICON_SIZE)
                            {
                                icon_animations_on[i++] = ch;
                            }
                        }
                    }
                }

                icon_animations_on[i] = '\0';
                icon_anim_on_len = i;

                // animations on section
                if (ch == '-')
                {
                    i = 0;

                    while ((ch = fp.read ()) != EOF)
                    {
                        if (ch != '\r' && ch != '\n')
                        {
                            if (ch == '*' || ch == '-')
                            {
                                break;
                            }

                            if (ch != ' ' && ch != '\t')
                            {
                                if (i < MAX_ICON_SIZE)
                                {
                                    icon_animations_off[i++] = ch;
                                }
                            }
                        }
                    }

                    icon_animations_off[i] = '\0';
                    icon_anim_off_len = i;
                }
                else
                {
                    icon_animations_off[0] = '\0';
                }
            }
            else
            {
                icon_animations_on[0] = '\0';
            }

            if (! strcmp (icon_name, name))
            {
                icon_found = 1;
                break;
            }
        }

        fp.close ();
    }

    LittleFS.end ();
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * BEFRISTETE DIAGNOSEHILFE ZU L175 - DER RUECKBAU GEHOERT ZUR MASSNAHME (Katalog C9c4)
 *
 * free_heap und max_free_block stehen zwar in /api/device_ready, aber NUR AUF ABRUF:
 * Wer nicht gerade fragt, sieht nichts, und nach einem Absturz ist der Zustand davor
 * verloren. L175 - der groesste zusammenhaengende Block faellt im Betrieb um 44 Prozent -
 * wurde ueberhaupt nur gefunden, weil zufaellig vor und nach einem Neustart gemessen
 * wurde. Diese Zeile schliesst genau diese Luecke.
 *
 * Sie traegt das Praefix "- ", das der STM als ESP8266_DEBUGMSG erkennt
 * (src/esp8266/esp8266.c:332). Kein STM-Logtext beginnt so: der Ring wird
 * ausschliesslich ueber "LOG " gefuellt (src/log/log.c:33), und von den dortigen
 * Formatzeichenketten faengt keine mit "- " an - geprueft per grep -a ueber src/**
 * am 04.10.2026. "- " heisst im Ring also: vom ESP.
 *
 * Hier stand bis zum 04.10.2026, die Zeile sei damit ueber /api/stm32_log lesbar.
 * Das war falsch, und das Praefix ist nicht der Grund: Den Ring fuellt AUSSCHLIESSLICH
 * stm32_log_append (), und das rief hier niemand auf. Gemessen wurden 36 Zeilen im
 * seriellen Mitschnitt gegen 0 Zeilen ueber die API (C14, L185). Die Zeile geht
 * deshalb unten ausdruecklich BEIDE Wege.
 *
 * Kosten: rund 26 Byte je Minute, also 0,4 Byte/s auf einer Bruecke mit 37 bis 80
 * Byte/s Grundlast (L141). Vernachlaessigbar - ABER NUR, SOLANGE SIE GEBRAUCHT WIRD.
 *
 * WEG DAMIT, sobald die Fragmentierung behoben und ueber mehrere Tage bestaetigt ist:
 * ESP_HEAP_LOG auf 0 setzen oder den Block ganz entfernen. Steht sie ungeprueft weiter
 * hier, traegt sie zu genau der Bruecken- und Speicherlast bei, die wir gerade senken -
 * dasselbe Muster wie "- new client", das jahrelang mitlief, bis es jemandem auffiel.
 *
 * Die Messzeile darf den Haufen nicht anfassen, sonst misst sie sich selbst. Der
 * Aufbau laeuft deshalb ueber snprintf in einen Stackpuffer und NICHT ueber String:
 * dessen SSO reicht nur bis 14 Zeichen, darueber wird in 16-Byte-Schritten
 * nachalloziert - genau die Fragmentierungsquelle aus L175.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#define ESP_HEAP_LOG                1                                       // 0 = aus. Rueckbau zu L175, siehe oben
#define ESP_HEAP_LOG_INTERVAL       60000UL                                 // msec

#if ESP_HEAP_LOG
// Lokaler Prototyp, zwingend: arduino-cli erzeugt fuer jede .ino-Funktion ohne eigenen
// Prototyp selbst einen - und zwar OHNE static (ESP-uclock.ino.cpp). "extern deklariert,
// spaeter static" ist ein Fehler, der Bau bricht ab. Er muss HIER stehen und nicht oben
// bei icon_info: ESP_HEAP_LOG wird erst in Zeile 442 definiert, dort waere #if noch 0.
static void           esp_heap_log (void);

static void
esp_heap_log (void)
{
    static unsigned long    last_millis = 0;
    static uint_fast8_t     pending     = 1;                                // die erste Zeile sofort, nicht erst nach einer Minute

    if (pending || (millis () - last_millis) >= ESP_HEAP_LOG_INTERVAL)      // Differenzbildung ist ueberlaufsicher
    {
        pending     = 0;
        last_millis = millis ();

        /* EINMAL formatieren, ZWEIMAL ausgeben: Mitschnitt und API duerfen nicht
         * auseinanderlaufen - eine zweite Formatierung waere eine zweite Wahrheit.
         * Laengste Form: "- heap free=4294967295 max=4294967295" = 37 Zeichen.
         */
        char line[48];

        snprintf (line, sizeof (line), "- heap free=%lu max=%lu",
                  (unsigned long) ESP.getFreeHeap (),
                  (unsigned long) ESP.getMaxFreeBlockSize ());

        Serial.println (line);
        Serial.flush ();
        stm32_log_append (line);                                            // C14 (L185): ohne diese Zeile bleibt
                                                                            // die Messung ueber /api/stm32_log
                                                                            // unsichtbar - das Praefix allein
                                                                            // genuegt nicht
    }
}
#endif

/*----------------------------------------------------------------------------------------------------------------------------------------
 * Pruefsumme der Bruecke (A32, specs/bruecke-wiederholung, Design 6.1/6.2)
 *
 * WOGEGEN SIE ANTRITT: Der Punkt als Quittung heisst "Zeile gelesen", nicht "Wert uebernommen"
 * (L233). Eine verstuemmelte Zeile wurde bis hierher angewandt UND quittiert -- der Sender sah
 * Erfolg. Belegt ist der Schaden in L205: Aus overlay[0].type = 2 wurde 14, gueltig aussehend,
 * ausserhalb des zulaessigen Bereichs 0..10, von keiner Pruefung bemerkt. Ein fehlender Wert
 * faellt auf, ein falscher nicht -- deshalb ist das der gefaehrlichere der beiden Fehler.
 *
 * Eine Zeile, deren Pruefsumme nicht stimmt, wird daher NICHT angewandt, sondern mit "!v"
 * abgelehnt. Der STM merkt sie daraufhin sofort zur Nachsendung vor und wartet nicht erneut
 * (src/vars/vars.c, var_send_buf()).
 *
 * RUECKWAERTSKOMPATIBEL: Eine Zeile OHNE Marke wird behandelt wie bisher -- angewandt und mit
 * dem Punkt quittiert. Solange der STM die Marke nicht anhaengt (das tut er erst mit gesetztem
 * cap_var_crc), ist dieser Zweig wirkungslos. Das ist die Zusage AK8.
 *
 * Die Rechnung steht ein zweites Mal im STM (C) -- das sind zwei Wahrheiten. Schiedsrichter ist
 * eine dritte, unabhaengig und VOR beiden entstandene Umsetzung mit festen Vektoren:
 * tools/checks/var-crc.c. Wer hier etwas aendert, rechnet gegen DIESE Vektoren und schreibt sie
 * nicht ab.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#define VAR_CRC_MARK        '*'                                             // Trenner vor der Pruefsumme
#define VAR_CRC_MARK_LEN    5                                               // '*' + vier Hexziffern

/* Prototypen von Hand, zwingend: arduino-cli erzeugt fuer jede .ino-Funktion ohne eigenen
 * Prototyp selbst einen -- und zwar OHNE static. "extern deklariert, spaeter static" ist ein
 * Fehler, der Bau bricht ab; genau daran scheiterte esp_heap_log() am 04.10.2026 (L177).
 */
static uint16_t         var_crc (const char * payload, size_t len);
static int              var_crc_hexval (char ch);
static void             var_crc_reject (unsigned int len);
static uint_fast8_t     var_crc_check_and_strip (char * parameters);

/* Fletcher-artig, ohne Tabelle und ohne Division. Die LAENGE ist der Startwert -- damit wirkt
 * sich eine Laengenaenderung aus, und genau die ist der belegte Schadensfall: Der Empfangsring
 * verliert zusammenhaengende Bloecke von rund 77 Byte (L141, L188).
 *
 * Die Laenge wird uebergeben und nicht mit strlen() geholt: Gerechnet wird ueber die Nutzlast
 * OHNE "var " und OHNE die Marke selbst, und die steht beim Pruefen noch in der Zeile.
 */
static uint16_t
var_crc (const char * payload, size_t len)
{
    uint8_t     sum1;
    uint8_t     sum2 = 0;
    size_t      i;

    sum1 = (uint8_t) len;

    for (i = 0; i < len; i++)
    {
        sum1 = (uint8_t) (sum1 + (uint8_t) payload[i]);
        sum2 = (uint8_t) (sum2 + sum1);
    }

    return (uint16_t) ((sum2 << 8) | sum1);
}

/* Strenge Hexziffer, -1 wenn keine. AUSDRUECKLICH NICHT htoi(): Das behandelt jedes
 * Nicht-Hexzeichen still als 0 (L232) -- eine Pruefung, die ihre eigene Eingabe zurechtbiegt,
 * prueft nichts. Grossbuchstaben werden mitgenommen, obwohl der STM "%04x" sendet: kostet drei
 * Zeilen und faengt den Tag, an dem dort "%04X" steht.
 */
static int
var_crc_hexval (char ch)
{
    if (ch >= '0' && ch <= '9')
    {
        return ch - '0';
    }

    if (ch >= 'a' && ch <= 'f')
    {
        return ch - 'a' + 10;
    }

    if (ch >= 'A' && ch <= 'F')
    {
        return ch - 'A' + 10;
    }

    return -1;
}

/* Eine abgewiesene Zeile festhalten -- aber nicht auf der Leitung.
 *
 * AUSSCHLIESSLICH stm32_log_append(), kein Serial.println und kein debugmsg: L109 beschreibt
 * genau die Mitkopplung, um die es hier geht -- eine Meldung ueber einen Uebertragungsfehler
 * legt Last auf dieselbe Leitung, deren Ueberlastung den Fehler erzeugt hat, und der RX-Ring
 * des STM ist 256 Byte gross und verwirft bei Ueberlauf still (uart-driver.h:698, kein
 * else-Zweig). Derselbe Entscheid wie bei var_cmd_reject() in vars.cpp (L237).
 *
 * Abrufbar ist die Zeile ueber /api/stm32_log. Das ist die Lehre aus C14/L185: Den Ring fuellt
 * ausschliesslich stm32_log_append(), ein Praefix allein genuegt nicht -- dort wurden 36 Zeilen
 * im seriellen Mitschnitt gegen 0 Zeilen ueber die API gemessen.
 *
 * Gedrosselt, weil der Ring nur STM32_LOG_LINES Zeilen fasst und auch die Diagnosezeilen traegt:
 * die ersten vier Faelle einzeln, danach jeder fuenfzigste. Der laufende Zaehler steht IN der
 * Zeile -- die Gesamtzahl geht also nicht verloren, auch wenn nur die letzte uebrig ist.
 *
 * KEIN Wert in der Zeile, nur Zahl und Laenge: Zeichenkettenvariablen tragen Hostnamen und
 * Zugangsdaten (A20/L115).
 */
static uint32_t     var_form_error_cnt = 0;                                 // abgewiesene var-Zeilen seit dem ESP-Start

static void
var_crc_reject (unsigned int len)
{
    var_form_error_cnt++;

    if (var_form_error_cnt <= 4 || (var_form_error_cnt % 50) == 0)
    {
        char line[48];                                                      // laengste Form: 35 Zeichen

        snprintf (line, sizeof (line), "- var abgewiesen #%lu len=%u",
                  (unsigned long) var_form_error_cnt, len);
        stm32_log_append (line);
    }
}

/* Marke erkennen, pruefen, abschneiden.
 *
 * Rueckgabe: 1 = Zeile darf angewandt werden (ohne Marke, oder Pruefsumme stimmte),
 *            0 = abgewiesen. Dann ist die Zeile UNVERAENDERT und wurde NICHT angewandt.
 *
 * Abgeschnitten wird VOR dem Aufruf von var_set_parameter(): Dessen Laengenpruefung aus L237
 * zaehlt sonst die fuenf Zeichen der Marke als Nutzlast mit und laesst eine zu kurze Zeile durch.
 *
 * Benannter Grenzfall: Eine Zeile, deren NUTZTEXT zufaellig auf '*' + vier Hexziffern endet,
 * waehrend der STM gar keine Marke anhaengt (vor Runde 3 oder nach einem Rueckrollen), wird als
 * falsch markiert gelesen und abgewiesen. Die Zeile geht dabei nicht verloren -- der STM sendet
 * sie auf "!v" hin nach und bekommt dasselbe Ergebnis, der alte Wert bleibt stehen. Das ist die
 * sichere Richtung: ein stehengebliebener Wert faellt beim naechsten Abgleich auf, ein still
 * beschaedigter nicht (L205). Betroffen waere nur freier Text (ON/xN/S) mit genau diesem Ende.
 */
static uint_fast8_t
var_crc_check_and_strip (char * parameters)
{
    size_t          len;
    size_t          payload_len;
    char *          mark;
    int             digit;
    int             i;
    uint_fast16_t   want;

    len = strlen (parameters);

    if (len < VAR_CRC_MARK_LEN)
    {
        return 1;                                                           // zu kurz fuer eine Marke -- wie bisher behandeln
    }

    mark = parameters + (len - VAR_CRC_MARK_LEN);

    if (*mark != VAR_CRC_MARK)
    {
        return 1;                                                           // keine Marke: rueckwaertskompatibel annehmen
    }

    want = 0;

    for (i = 1; i < VAR_CRC_MARK_LEN; i++)
    {
        digit = var_crc_hexval (mark[i]);

        if (digit < 0)
        {
            return 1;                                                       // '*' ohne vier Hexziffern dahinter ist Nutztext
        }

        want = (uint_fast16_t) ((want << 4) | (uint_fast16_t) digit);
    }

    payload_len = len - VAR_CRC_MARK_LEN;

    if (want != (uint_fast16_t) var_crc (parameters, payload_len))
    {
        var_crc_reject ((unsigned int) payload_len);
        return 0;
    }

    *mark = '\0';                                                           // Marke abschneiden, siehe Kopfkommentar

    return 1;
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * main loop
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
loop() 
{
    static char cmd_buffer[CMD_BUFFER_SIZE];
    static int  cmd_len = 0;
    static int  cmd_trunc = 0;                                              // Zeile war laenger als der Puffer

    wifi_check_if_started ();
    http_server_loop ();
    udp_server_loop ();
    ntp_poll_time ();                                                       // poll NTP

#if ESP_HEAP_LOG
    esp_heap_log ();                                                    // befristet, siehe Kommentar oben (L175/L177)
#endif

    while (Serial.available())
    {
        char ch = Serial.read ();
    
        if (ch == '\n')
        {
            if (cmd_trunc && cmd_len > 0)                                   // gekuerzt? Marke statt letztem Zeichen,
            {                                                               // sonst sieht die Zeile vollstaendig aus
                cmd_buffer[cmd_len - 1] = LOG_TRUNC_MARK;
            }

            cmd_buffer[cmd_len] = '\0';

            if (! strncmp (cmd_buffer, "var ", 4))
            {
                char *          parameter;
                uint_fast8_t    ok;

                parameter = cmd_buffer + 4;

                /* Erst pruefen, dann anwenden (A32, Design 6.2). Die Marke schneidet
                 * var_crc_check_and_strip() dabei ab -- sie darf var_set_parameter() nie
                 * erreichen, sonst zaehlt dessen Laengenpruefung (L237) fuenf Zeichen mit,
                 * die nicht zur Nutzlast gehoeren.
                 */
                ok = var_crc_check_and_strip (parameter);

                if (ok)
                {
                    /* Rueckgabewert seit L237: 1 = formal verwertbar, 0 = verworfen, weil die
                     * Zeile zu kurz fuer ihre Kommandoart war. AUCH DAS wird abgelehnt -- sonst
                     * verhielten sich Laengen- und Pruefsummenverwurf unterschiedlich, und der
                     * STM saehe nur den einen der beiden Faelle und sendete nur ihn nach.
                     *
                     * Weiterhin NICHT bestaetigt wird die Uebernahme: Eine formal gueltige Zeile
                     * mit unbekanntem Kommandobuchstaben liefert ebenfalls 1. Das ist der offene
                     * Punkt L233 und wird hier nicht geloest -- eine Wiederholung aenderte daran
                     * nichts.
                     */
                    ok = var_set_parameter (parameter);
                }

                if (ok)
                {
                    Serial.println (".");                                   // "silent" OK
                }
                else
                {
                    Serial.println ("!v");                                  // abgelehnt und NICHT angewandt; der STM merkt
                }                                                           // das Kommando sofort zur Nachsendung vor

                Serial.flush ();
            }
            else if (! strcmp (cmd_buffer, "time"))
            {
                if (! wifi_ap_mode)
                {
                    ntp_get_time ();
                }
            }
            else if (! strncmp (cmd_buffer, "time ", 5))
            {
                if (! wifi_ap_mode)
                {
                    int       syntax_ok = false;
                    char *    p = cmd_buffer + 5;
                    char *    pp;

                    if (*p == '"')
                    {
                        pp = strchr (p + 1, '"');

                        if (pp)
                        {
                            *pp = '\0';

                            ntp_get_time (p + 1);
                            syntax_ok = true;
                        }
                    }

                    if (! syntax_ok)
                    {
                        Serial.println ("ERROR syntax error");
                        Serial.flush ();
                    }
                }
            }
            else if (! strncmp (cmd_buffer, "ap ", 3))
            {
                int     syntax_ok = false;
                char *  ssid;
                char *  key;
                char *  p = cmd_buffer + 3;
                char *  pp;

                if (*p == '"')
                {
                    pp = strchr (p + 1, '"');

                    if (pp)
                    {
                        *pp = '\0';
                        ssid = p + 1;
                        p = pp + 1;

                        if (*p == ',' && *(p + 1) == '"')
                        {
                            pp = strchr (p + 2, '"');

                            if (pp)
                            {
                                syntax_ok = true;
                                *pp = '\0';
                                key = p + 2;

                                wifi_ap (ssid, key);
                            }
                        }
                    }
                }

                if (! syntax_ok)
                {
                    Serial.println ("ERROR syntax error");
                    Serial.flush ();
                }
            }
            else if (! strncmp (cmd_buffer, "cap ", 4))
            {
                int     syntax_ok = false;
                char *  ssid;
                char *  key;
                char *  p = cmd_buffer + 4;
                char *  pp;

                if (*p == '"')
                {
                    pp = strchr (p + 1, '"');

                    if (pp)
                    {
                        *pp = '\0';
                        ssid = p + 1;
                        p = pp + 1;
                        if (*p == ',' && *(p + 1) == '"')
                        {
                            pp = strchr (p + 2, '"');

                            if (pp)
                            {
                                syntax_ok = true;
                                *pp = '\0';
                                key = p + 2;

                                wifi_connect (ssid, key, true);
                            }
                        }
                    }
                }

                if (! syntax_ok)
                {
                    Serial.println ("ERROR syntax error");
                    Serial.flush ();
                }
            }
            else if (! strncmp (cmd_buffer, "weather ", 8))
            {
                if (! wifi_ap_mode)
                {
                    int     syntax_ok = false;
                    char *  appid;
                    char *  city;
                    char *  lon;
                    char *  lat;
                    char *  p = cmd_buffer + 8;
                    char *  pp;

                    if (*p == '"')
                    {
                        pp = strchr (p + 1, '"');

                        if (pp)
                        {
                            *pp = '\0';
                            appid = p + 1;
                            p = pp + 1;

                            if (*p == ',' && *(p + 1) == '"')
                            {
                                pp = strchr (p + 2, '"');

                                if (pp)
                                {
                                    *pp = '\0';

                                    if (*(pp + 1))                                       // "appid","lon","lat"
                                    {
                                        lon = p + 2;
                                        p = pp + 1;

                                        if (*p == ',' && *(p + 1) == '"')
                                        {
                                            pp = strchr (p + 2, '"');

                                            if (pp)
                                            {
                                                *pp = '\0';
                                                lat = p + 2;
                                                syntax_ok = true;
                                                get_weather (appid, lon, lat);
                                            }
                                        }
                                    }
                                    else                                                // "appid","city"
                                    {
                                        city = p + 2;
                                        syntax_ok = true;
                                        get_weather (appid, city);
                                    }
                                }
                            }
                        }
                    }

                    if (! syntax_ok)
                    {
                        Serial.println ("ERROR syntax error");
                        Serial.flush ();
                    }
                }
            }
            else if (! strncmp (cmd_buffer, "weather_fc ", 11))
            {
                if (! wifi_ap_mode)
                {
                    int     syntax_ok = false;
                    char *  appid;
                    char *  city;
                    char *  lon;
                    char *  lat;
                    char *  p = cmd_buffer + 11;
                    char *  pp;

                    if (*p == '"')
                    {
                        pp = strchr (p + 1, '"');

                        if (pp)
                        {
                            *pp = '\0';
                            appid = p + 1;
                            p = pp + 1;

                            if (*p == ',' && *(p + 1) == '"')
                            {
                                pp = strchr (p + 2, '"');

                                if (pp)
                                {
                                    *pp = '\0';

                                    if (*(pp + 1))                                       // "appid","lon","lat"
                                    {
                                        lon = p + 2;
                                        p = pp + 1;

                                        if (*p == ',' && *(p + 1) == '"')
                                        {
                                            pp = strchr (p + 2, '"');

                                            if (pp)
                                            {
                                                *pp = '\0';
                                                lat = p + 2;
                                                syntax_ok = true;
                                                get_weather_fc (appid, lon, lat);
                                            }
                                        }
                                    }
                                    else                                                // "appid","city"
                                    {
                                        city = p + 2;
                                        syntax_ok = true;
                                        get_weather_fc (appid, city);
                                    }
                                }
                            }
                        }
                    }

                    if (! syntax_ok)
                    {
                        Serial.println ("ERROR syntax error");
                        Serial.flush ();
                    }
                }
            }
            else if (! strncmp (cmd_buffer, "wicon ", 6))
            {
                if (! wifi_ap_mode)
                {
                    int     syntax_ok = false;
                    char *  appid;
                    char *  city;
                    char *  lon;
                    char *  lat;
                    char *  p = cmd_buffer + 6;
                    char *  pp;

                    if (*p == '"')
                    {
                        pp = strchr (p + 1, '"');

                        if (pp)
                        {
                            *pp = '\0';
                            appid = p + 1;
                            p = pp + 1;

                            if (*p == ',' && *(p + 1) == '"')
                            {
                                pp = strchr (p + 2, '"');

                                if (pp)
                                {
                                    *pp = '\0';

                                    if (*(pp + 1))                                       // "appid","lon","lat"
                                    {
                                        lon = p + 2;
                                        p = pp + 1;

                                        if (*p == ',' && *(p + 1) == '"')
                                        {
                                            pp = strchr (p + 2, '"');

                                            if (pp)
                                            {
                                                *pp = '\0';
                                                lat = p + 2;
                                                syntax_ok = true;
                                                get_weather_icon (appid, lon, lat);
                                            }
                                        }
                                    }
                                    else                                                // "appid","city"
                                    {
                                        city = p + 2;
                                        syntax_ok = true;
                                        get_weather_icon (appid, city);
                                    }
                                }
                            }
                        }
                    }

                    if (! syntax_ok)
                    {
                        Serial.println ("ERROR syntax error");
                        Serial.flush ();
                    }
                }
            }
            else if (! strncmp (cmd_buffer, "wicon_fc ", 9))
            {
                if (! wifi_ap_mode)
                {
                    int     syntax_ok = false;
                    char *  appid;
                    char *  city;
                    char *  lon;
                    char *  lat;
                    char *  p = cmd_buffer + 9;
                    char *  pp;

                    if (*p == '"')
                    {
                        pp = strchr (p + 1, '"');

                        if (pp)
                        {
                            *pp = '\0';
                            appid = p + 1;
                            p = pp + 1;

                            if (*p == ',' && *(p + 1) == '"')
                            {
                                pp = strchr (p + 2, '"');

                                if (pp)
                                {
                                    *pp = '\0';

                                    if (*(pp + 1))                                       // "appid","lon","lat"
                                    {
                                        lon = p + 2;
                                        p = pp + 1;

                                        if (*p == ',' && *(p + 1) == '"')
                                        {
                                            pp = strchr (p + 2, '"');

                                            if (pp)
                                            {
                                                *pp = '\0';
                                                lat = p + 2;
                                                syntax_ok = true;
                                                get_weather_icon_fc (appid, lon, lat);
                                            }
                                        }
                                    }
                                    else                                                // "appid","city"
                                    {
                                        city = p + 2;
                                        syntax_ok = true;
                                        get_weather_icon_fc (appid, city);
                                    }
                                }
                            }
                        }
                    }

                    if (! syntax_ok)
                    {
                        Serial.println ("ERROR syntax error");
                        Serial.flush ();
                    }
                }
            }
            else if (! strcmp (cmd_buffer, "wps"))
            {
                wifi_wps ();
            }
            else if (! strncmp (cmd_buffer, "disp ", 5))
            {
                int     syntax_ok = false;
                char *  linep;
                char *  colsp;
                char *  p = cmd_buffer + 5;
                char *  pp;

                if (*p == '"')
                {
                    pp = strchr (p + 1, '"');

                    if (pp)
                    {
                        *pp = '\0';
                        linep = p + 1;
                        p = pp + 1;

                        if (*p == ',' && *(p + 1) == '"')
                        {
                            pp = strchr (p + 2, '"');

                            if (pp)
                            {
                                syntax_ok = true;
                                *pp = '\0';
                                colsp = p + 2;

                                display_layout_values (atoi (linep), atoi (colsp));
                            }
                        }
                    }
                }

                if (! syntax_ok)
                {
                    Serial.println ("ERROR syntax error");
                    Serial.flush ();
                }
            }
            else if (! strcmp (cmd_buffer, "tabinfo"))
            {
                tables_info ();
            }
            else if (! strncmp (cmd_buffer, "tabillu ", 8))
            {
                int     syntax_ok = false;
                char *  p = cmd_buffer + 8;
                char *  pp;

                if (*p == '"')
                {
                    pp = strchr (p + 1, '"');

                    if (pp)
                    {
                        *pp = '\0';
                        tables_illumination (atoi (p + 1));
                        syntax_ok = true;
                    }
                }
                if (! syntax_ok)
                {
                    Serial.println ("ERROR syntax error");
                    Serial.flush ();
                }
            }
            else if (! strncmp (cmd_buffer, "tabh ", 5))
            {
                int     syntax_ok = false;
                char *  modep;
                char *  idxp;
                char *  p = cmd_buffer + 5;
                char *  pp;

                if (*p == '"')
                {
                    pp = strchr (p + 1, '"');

                    if (pp)
                    {
                        *pp = '\0';
                        modep = p + 1;
                        p = pp + 1;
                        if (*p == ',' && *(p + 1) == '"')
                        {
                            pp = strchr (p + 2, '"');

                            if (pp)
                            {
                                syntax_ok = true;
                                *pp = '\0';
                                idxp = p + 2;

                                tables_hours (atoi (modep), atoi (idxp));
                            }
                        }
                    }
                }

                if (! syntax_ok)
                {
                    Serial.println ("ERROR syntax error");
                    Serial.flush ();
                }
            }
            else if (! strncmp (cmd_buffer, "tabm ", 5) || ! strncmp (cmd_buffer, "tabt ", 5))
            {
                int     syntax_ok = false;
                char *  modep;
                char *  idxp;
                char *  p = cmd_buffer + 5;
                char *  pp;

                if (*p == '"')
                {
                    pp = strchr (p + 1, '"');

                    if (pp)
                    {
                        *pp = '\0';
                        modep = p + 1;
                        p = pp + 1;
                        if (*p == ',' && *(p + 1) == '"')
                        {
                            pp = strchr (p + 2, '"');

                            if (pp)
                            {
                                syntax_ok = true;
                                *pp = '\0';
                                idxp = p + 2;

                                if (! strncmp (cmd_buffer, "tabm ", 5))
                                {
                                    tables_minutes (atoi (modep), atoi (idxp), "TABM");
                                }
                                else
                                {
                                    tables_minutes (atoi (modep), atoi (idxp), "TABT");
                                }
                            }
                        }
                    }
                }

                if (! syntax_ok)
                {
                    Serial.println ("ERROR syntax error");
                    Serial.flush ();
                }
            }
#if 0 // yet not used
            else if (! strncmp (cmd_buffer, "file-open ", 10))
            {
                char *  p = cmd_buffer + 10;
                char *  pp;
                char *  fname;
                int     size;

                if (*p == '"' && (pp = strchr (p + 1, '"')) != (char *) NULL)
                {
                    *pp = '\0';
                    fname = p + 1;

                    LittleFS.begin ();
                    fp = LittleFS.open (fname, "r");
    
                    Serial.print ("OPEN ");
    
                    if (fp)
                    {
                        size = fp.size();
                    }
                    else
                    {
                        size = -1;
                    }
    
                    if (size <= 0)
                    {
                        if (fp)
                        {
                            fp.close ();
                            fp = (File) 0;
                        }
                        LittleFS.end ();
                    }

                    Serial.println (size);
                }
                else
                {
                    Serial.println ("ERROR syntax error");
                }
                Serial.flush ();
            }
            else if (! strcmp (cmd_buffer, "file-close"))
            {
                if (fp)
                {
                    fp.close ();
                    fp = (File) 0;
                    LittleFS.end ();
                }

                Serial.println ("CLOSE");
                Serial.flush ();
            }
            else if (! strcmp (cmd_buffer, "file-read"))
            {
                int idx;
                int ch;
                Serial.print ("FILE ");

                if (fp)
                {
                    for (idx = 0; idx < 16; idx++)
                    {
                        ch = fp.read ();

                        if (ch < 0)
                        {
                            break;
                        }
                        Serial.printf ("%02x", ch);
                    }
                }

                Serial.println ("");
                Serial.flush ();
            }
#endif
            else if (! strncmp (cmd_buffer, "icon ", 5))            // open icon file and send infos about icon
            {
                int     syntax_ok = false;
                char *  fname;
                char *  icon_name;
                char *  p = cmd_buffer + 5;
                char *  pp;

                if (*p == '"')
                {
                    pp = strchr (p + 1, '"');

                    if (pp)
                    {
                        *pp = '\0';
                        fname = p + 1;
                        p = pp + 1;

                        if (*p == ',' && *(p + 1) == '"')
                        {
                            pp = strchr (p + 2, '"');

                            if (pp)
                            {
                                char answer[32];

                                syntax_ok = true;
                                *pp = '\0';
                                icon_name = p + 2;

                                icon_info (fname, icon_name);
                                sprintf (answer, "ICON %02x%02x%04x%04x%04x", icon_rows, icon_cols, icon_colors_len, icon_anim_on_len, icon_anim_off_len);

                                colors_pos = 0;
                                anim_on_pos = 0;
                                anim_off_pos = 0;
                                Serial.println (answer);
                            }
                        }
                    }
                }

                if (! syntax_ok)
                {
                    Serial.println ("ERROR syntax error");
                    Serial.flush ();
                }

                Serial.flush ();
            }
            else if (! strcmp (cmd_buffer, "icon"))            // send icon data in 16-byte blocks
            {
                int idx;
                int ch;
                Serial.print ("ICON ");

                if (icon_found)
                {
                    for (idx = 0; idx < 16; idx++)
                    {
                        if (colors_pos < icon_colors_len)
                        {
                            ch = icon_colors[colors_pos];
                            colors_pos++;
                        }
                        else if (anim_on_pos < icon_anim_on_len)
                        {
                            ch = icon_animations_on[anim_on_pos];
                            anim_on_pos++;
                        }
                        else if (anim_off_pos < icon_anim_off_len)
                        {
                            ch = icon_animations_off[anim_off_pos];
                            anim_off_pos++;
                        }
                        else
                        {
                            break;
                        }
                        Serial.printf ("%02x", ch);
                    }
                }

                Serial.println ("");
                Serial.flush ();
            }
            else if (! strncmp (cmd_buffer, "LOG ", 4))
            {
                stm32_log_append (cmd_buffer + 4);
            }

            cmd_buffer[0] = '\0';
            cmd_len = 0;
            cmd_trunc = 0;
        }
        else
        {
            if (ch != '\r')
            {
                if (cmd_len < CMD_BUFFER_SIZE - 1)
                {
                    cmd_buffer[cmd_len++] = ch;
                }
                else
                {
                    cmd_trunc = 1;                                          // Zeichen faellt weg, Marke folgt am Zeilenende
                }
            }
        }
    }
}
