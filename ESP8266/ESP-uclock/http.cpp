/*----------------------------------------------------------------------------------------------------------------------------------------
 * http.cpp - http server
 *
 * Copyright (c) 2016-2025 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <Updater.h>
#include <WString.h>
#include <FS.h>
#include <LittleFS.h>
#include "base.h"
#include "vars.h"
#include "wifi.h"
#include "version.h"
#include "http.h"
#include "httpclient.h"
#include "stm32flash.h"
#include "tables.h"
#include "eepromdata.h"

#define WCLOCK24H   1

const char *                wdays_en[7] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" };
const char *                wdays_de[7] = { "So", "Mo", "Di", "Mi", "Do", "Fr", "Sa" };

WiFiServer                  http_server(80);                                        // create an instance of the server on Port 80
static WiFiClient           http_client;

static const char *         pgm_name = "unknown Clock";
static const char *         hardware = "unknown";
static uint_fast16_t        hardware_configuration = 0xFFFF;

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * Debugausgaben auf die STM-UART
 *
 * Der ESP hat nur EINEN vollwertigen UART, und das ist die Bruecke zum STM. Jede
 * Serial-Ausgabe landet deshalb zwangslaeufig im 256-Byte-Empfangsring des STM, und
 * der verwirft bei Ueberlauf still (uart-driver.h, kein else-Zweig).
 *
 * Je Request standen hier zwei unbedingte Zeilen zu zusammen rund 105 Byte:
 *      "- new client from <ip>"                rund 35 Byte
 *      "- request <ip> [<browser>]: <req>"     rund 70 Byte
 *
 * Die erste traegt keine Information, die die zweite nicht schon enthaelt: dieselbe
 * IP, derselbe Request. Der STM wertet ohnehin keine von beiden aus -- fuehrendes
 * "- " wird zu ESP8266_DEBUGMSG, und schedule_esp8266_messages() hat dafuer keinen
 * case. Sie kostet reine Abholzeit und hat nachweislich schon Ausgaben zerrissen
 * (BEFUNDE.md L13: "- request 192.168.x.y- new client from 192.168.x.yz").
 *
 * Darum ist sie abschaltbar statt geloescht: Fuer die Fehlersuche
 * HTTP_DEBUG_CLIENT_LOG auf 1 setzen und neu bauen, im Dauerbetrieb bleibt sie aus
 * (A21 / BEFUNDE.md L141, L144). Die zweite Zeile bleibt unbedingt -- sie ist das
 * einzige Protokoll darueber, WAS angefragt wurde.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define HTTP_DEBUG_CLIENT_LOG                       0

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * firmware update parameters
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define DEFAULT_UPDATE_HOST                         "uclock.de"
#define DEFAULT_UPDATE_PATH                         "update"

#define ESP_WORDCLOCK_TXT                           "ESP-WordClock.txt"             // avaliable version of ESP8266 firmware
#define APP_VERSION_TXT                             "app-version.txt"               // available version of app files
#define ESP_WORDCLOCK_BIN                           "ESP-WordClock-4M.bin"          // name of ES8266 firmware bin file

#define RELEASENOTE_HTML                            "releasenote.html"              // release notes
#define WC_TXT                                      "wc.txt"                        // avaliable version of STM32 firmware
#define WC_LIST_TXT                                 "wc-list.txt"                   // list of available STM32 firmware files
#define WC_TABLES_LIST_TXT                          "wc-list-tables.txt"            // list of available layout table files

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * http parameters
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MAX_LINE_LEN                                256                             // max. length of line_buffer
#define MAX_PATH_LEN                                20                              // max. length of path
#define MAX_HTTP_PARAMS                             16                              // max. number of http parameters

typedef struct
{
    char *  name;
    char *  value;
} HTTP_PARAMETERS;

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * globals
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static HTTP_PARAMETERS                              http_parameters[MAX_HTTP_PARAMS];
static int                                          bgcolor_cnt;

#define MAX_KEY_LEN                                 64
#define MAX_IP_LEN                                  15
#define MAX_TIMEZONE_LEN                            3
#define MAX_DATE_LEN                                10
#define MAX_TIME_LEN                                5
#define MAX_BRIGHTNESS_LEN                          2
#define MAX_COLOR_VALUE_LEN                         2
#define MAX_TEMP_CORR_LEN                           3
#define MAX_MINUTE_INTERVAL_LEN                     2
#define MAX_TICKER_DECELERATION_LEN                 3
#define MAX_RAINBOW_DECELERATION_LEN                3
#define MAX_RAW_VALUE_LEN                           5
#define MAX_ANIMATION_DECELERATION_LEN              2
#define MAX_COLOR_ANIMATION_DECELERATION_LEN        2
#define MAX_AMBILIGHT_MODE_DECELERATION_LEN         2

#define MAIN_HEADER_COLS                            2
#define DATETIME_HEADER_COLS                        6
#define TICKER_HEADER_COLS                          2
#define NETWORK_HEADER_COLS                         4
#define NETWORK_HEADER_COLS2                        3
#define WEATHER_HEADER_COLS                         3
#define DISPLAY_HEADER_COLS                         3
#define OVERLAY_HEADER_COLS                         10
#define ANIMATION_HEADER_COLS                       3
#define ANIMATION_DECELERATION_HEADER_COLS          5
#define COLOR_ANIMATION_DECELERATION_HEADER_COLS    4
#define AMBILIGHT_MODE_DECELERATION_HEADER_COLS     4
#define TIMERS_HEADER_COLS                          8
#define ALARM_TIMERS_HEADER_COLS                    7
#define UPDATE_HEADER_COLS                          2

#define DFPLAYER_HEADER_COLS                        3
#define DFPLAYER_SILENCE_COLS                       4

/* Antwort- und zugleich Leseblockgroesse der Dateiauslieferung: genau 2 x TCP_MSS.
 *
 * Der Build bindet die lwIP-Variante "v2 Lower Memory" ein (ip=lm2f in ESP_FQBN,
 * Makefile:11). Die Variante legt -DTCP_MSS=536 auf die Kommandozeile jedes
 * Uebersetzungslaufs (boards.txt, generic.menu.ip.lm2f.build.lwip_flags) und bindet
 * gegen liblwip2-536-feat. Daraus ergibt sich TCP_SND_BUF = 2 * TCP_MSS = 1072
 * (lwipopts.h:1326). MEHR je Schreibvorgang bringt nichts: ClientContext::_write_some()
 * schreibt hoechstens tcp_sndbuf() Byte am Stueck und wartet fuer den Rest ohnehin auf
 * die Quittung.
 *
 * Warum nicht mehr 1024: 1024 / 536 = 1,91. Jeder Block zerfiel in 536 + 488, das
 * zweite Segment war nie voll. Am Geraet gemessen (03.10.2026, fuenf vollstaendige
 * Abrufe): app.js.gz ging in 311 Paketen hinaus, erwartet waren 242. Mit 1072 ist
 * jeder Block genau zwei volle Segmente; 129'477 / 536 = 242.
 *
 * TCP_MSS ist dabei die Obergrenze, nicht die Zusicherung: lwIP sendet mit
 * min (TCP_MSS, der vom Gegenueber angebotenen MSS). Bietet eine Gegenstelle weniger
 * als 536 an, zerfaellt ein Block in mehr Segmente - nie in groessere, denn ueber die
 * zur Uebersetzungszeit festgelegten 536 kommt diese lwIP-Bibliothek nicht hinaus.
 * Der Wert ist damit in jedem Fall korrekt, im Regelfall exakt passend.
 *
 * Kosten: 48 Byte mehr im statischen Bereich. Nichts vom Heap - im Betrieb stehen nur
 * rund 5'400 Byte frei, groesster zusammenhaengender Block 4'648 (/api/device_ready).
 */
#ifndef TCP_MSS
#error "TCP_MSS ist nicht definiert - lwIP-Variante im FQBN pruefen (ip=lm2f)"
#endif

#define MAX_HTTP_RESPONSE_LEN                       (2 * TCP_MSS)   // 1072 bei ip=lm2f, genau TCP_SND_BUF
static char     http_response[MAX_HTTP_RESPONSE_LEN + 1];
static int      http_response_len = 0;

/* Auslieferungsdiagnose: Antwortbyte, die nicht in die Verbindung gelangt sind.
 * Kostet 11 Byte im statischen Bereich, nichts vom Heap. http_write_broken gilt je
 * Verbindung und wird beim Annehmen in http_server_loop () zurueckgesetzt; die vier
 * Zaehler laufen seit dem Start und stehen in /api/device_ready.
 */
static uint32_t     http_write_lost_bytes  = 0;
static uint16_t     http_write_lost_blocks = 0;
static uint16_t     http_write_lost_gone   = 0;     // C25/L270: verlorene Bloecke, Gegenstelle hatte schon abgebaut (Tab zu, Abbruch)
static uint16_t     http_write_lost_failed = 0;     // C25/L270: verlorene Bloecke bei stehender Verbindung - das Geraet bekam nichts los
static uint_fast8_t http_write_broken      = 0;

/* Verbindungen, die angenommen wurden, aber nie eine Anfrage brachten. Zwei
 * getrennte Zaehler, weil die Ursachen verschieden sind: "timeout" heisst, die
 * Gegenstelle hielt die Verbindung und schwieg (Chrome oeffnet Verbindungen auf
 * Vorrat), "abort" heisst, sie war beim Warten schon wieder weg (AbortController
 * der PWA). Zusammen 4 Byte im statischen Bereich; sie ersetzen eine unbedingte
 * Serial-Zeile von rund 30 Byte JE verworfener Verbindung auf der STM-UART.
 */
static uint16_t     http_no_request_timeouts = 0;
static uint16_t     http_no_request_aborts   = 0;

#define HTTP_FIRST_LINE_TIMEOUT                 250                     // msec Wartezeit auf die erste Anfragezeile

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display flags:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define DISPLAY_FLAGS_NONE                          0x00                    // no display flag
#define DISPLAY_FLAGS_PERMANENT_IT_IS               0x01                    // show "ES IST" permanently
#define DISPLAY_FLAGS_SYNC_AMBILIGHT                0x02                    // synchronize display and ambilight
#define DISPLAY_FLAGS_SYNC_CLOCK_MARKERS            0x04                    // synchronize display and clock markers
#define DISPLAY_FLAGS_FADE_CLOCK_SECONDS            0x08                    // fade clock seconds

/*--------------------------------------------------------------------------------------------------------------------------------------
 * possible modes of ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
#define ESP8266_CLIENT_MODE                         0
#define ESP8266_AP_MODE                             1

#define ESP8266_MAX_FIRMWARE_SIZE                   16
#define ESP8266_MAX_ACCESSPOINT_SIZE                32
#define ESP8266_MAX_IPADDRESS_SIZE                  16
#define ESP8266_MAX_HTTP_GET_PARAM_SIZE             256
#define ESP8266_MAX_CMD_SIZE                        32
#define ESP8266_MAX_TIME_SIZE                       16

#define MAX_COLOR_STEPS                             64

/*--------------------------------------------------------------------------------------------------------------------------------------
 * max update filename len
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
#define MAX_UPDATE_FILENAME_LEN                     64

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * pwa parameters
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define PWA_PREFIX                                  "/app"
#define PWA_INDEX_FILE                              "app-index.html"

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * Fehlercodes der JSON-Antworten der Setter. Sie stehen hier beisammen, damit nicht jede
 * Funktion eine eigene Nummerierung erfindet.
 *
 * DIE PWA WERTET DIE KENNUNG AUS, nicht nur "ok". Hier stand bis zum 05.10.2026 das
 * Gegenteil ("die PWA wertet nur ok aus, der Code dient der Diagnose am Geraet"), und das
 * ist eine falsche Zusicherung im Quelltext, dieselbe Gattung wie L274: describeApiError()
 * uebersetzt jede der Kennungen 1 bis 6 in einen eigenen deutschen Satz und haengt den
 * "detail"-Text woertlich in Klammern dahinter. Eine neue Kennung braucht deshalb ihren
 * Eintrag in BEIDEN i18n-Tabellen der PWA, sonst steht dort der Rueckfalltext; und wer
 * einen detail-Text aendert, aendert, was der Nutzer liest.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define HTTP_API_ERROR_MISSING_VALUE                1
#define HTTP_API_ERROR_OUT_OF_RANGE                 2
#define HTTP_API_ERROR_TOO_SHORT                    3
#define HTTP_API_ERROR_INVALID_DATE                 4
/* Nicht der Request ist falsch, sondern eine Voraussetzung am Geraet fehlt - etwa ein
 * Wetterabruf ohne Schluessel (L54). Die PWA kennt die Kennung noch nicht und zeigt
 * dann ihren eigenen Text samt "detail"; das ist richtiger als eine der vier
 * vorhandenen Uebersetzungen, die alle von einem falschen Feld sprechen.
 */
#define HTTP_API_ERROR_NOT_CONFIGURED               5

/* Die angeforderte Datei gibt es nicht. Eingefuehrt fuer fs_show (Befund L187 / C16):
 * Dort war "Datei fehlt" bis hierher von "Datei leer" nicht zu unterscheiden, weil beide
 * Faelle einen leeren Rumpf lieferten. Die Kennung erweitert den API-Vertrag -- die PWA
 * muss sie kennen (Task 1.5).
 */
#define HTTP_API_ERROR_NOT_FOUND                    6

/* Die Aktion selbst ist am Geraet fehlgeschlagen, obwohl der Request stimmte und das Ziel
 * existiert - etwa LittleFS.remove() mit false (Ent-4, AKF.7). Bis dahin trug dieser Fall
 * die Kennung 6 und sah fuer die PWA aus wie "Datei gibt es nicht". Die PWA kennt die
 * Kennung noch nicht und zeigt ihren Rueckfalltext samt "detail" (describeApiError()).
 */
#define HTTP_API_ERROR_ACTION_FAILED                7

/* Die Datei existiert, ist aber 0 Byte gross (C27/L271). fs_show lieferte sie als
 * 200 OK mit leerem Rumpf aus, und eine leere Datei ist der belegte Weisschirm-Fall der
 * Architektur-Invariante - also ein eigener, benannter Zustand statt eines leeren Rumpfs.
 */
#define HTTP_API_ERROR_FILE_EMPTY                   8

/* Hoechster Wochentag, den die Masken NIGHT_TIME_FROM_DAY_MASK/TO_DAY_MASK und
 * ALARM_TIME_FROM_DAY_MASK/TO_DAY_MASK tragen. Die Masken haben 3 Bit, also 0..7 -
 * belegt sind aber nur So..Sa, also 0..6. Stand bis C17/L197 unmittelbar vor
 * http_api_timer_set_common(); seitdem braucht ihn auch http_api_dfplayer_alarm_set(),
 * und das steht weiter oben in der Datei.
 */
#define HTTP_MAX_WEEKDAY                            6

static void             http_json_ok ();
static void             http_json_error (unsigned int error_code, const char * detail);
static uint_fast8_t     http_get_int_param (const char * name, int * valuep);
static uint_fast8_t     http_get_opt_int_param (const char * name, int * valuep, int lo, int hi);
static uint_fast8_t     http_get_string_param (const char * name, char ** valuep);
static uint_fast8_t     http_strvar_len_ok (const char * value, unsigned int maxlen);
static uint_fast8_t     http_check_strvar_len (const char * name, const char * value, unsigned int maxlen);
static void             http_json_error_range (const char * name, int lo, int hi);
static uint_fast8_t     http_days_in_month (int year, int month);
static uint_fast8_t     http_get_on_off_value (const char * param, uint_fast8_t current_value);
static int              http_get_on_off_required (const char * param);
static void             http_build_stm32_default_filename (char * stm32_default_filename, size_t max_len, const char ** filter);
static int              http_api_stm32_log ();
static int              http_api_stm32_log_clear ();
static int              http_api_remote_stm32_flash ();
static int              http_api_remote_esp_update ();
static int              http_api_device_ready ();
static int              http_api_reconnect_probe ();
static void             http_json_send_remote_update_support_fields ();
static void             http_json_send_remote_update_url_fields ();
static char *           http_get_param (const char * name);
static int              http_api_settings_xml ();
static int              http_api_display_power ();
static int              http_api_ambilight_power ();
static int              http_api_power_status ();
static int              http_api_update_progress ();
static bool             http_fs_file_exists_and_nonempty (const char * filename);
static uint_fast8_t     http_filename_matches (const char * actual, const char * expected);
static uint_fast8_t     http_local_stm32_filename_matches (const char * actual);
static uint_fast8_t     http_remote_stm32_filename_matches (const char * actual);
static uint_fast8_t     http_table_download_filename_matches (const char * actual);
static void             http_remove_table_family_files (const char * keep_filename);
static int              http_fetch_remote_line (const char * host, const char * path, const char * filename, char * buffer, size_t buffer_len);
static bool             app_asset_filename (const char * asset_path, char * filename, size_t maxlen);
static uint_fast8_t     http_app_asset_supports_gzip (const char * asset_path);
static bool             app_asset_storage_filename (const char * asset_path, uint_fast8_t gzip_encoded, char * filename, size_t maxlen);
static uint_fast8_t     http_find_stored_app_asset_filename (const char * asset_path, char * filename, size_t maxlen, uint_fast8_t * gzip_encoded);
static const char *     http_find_app_install_asset (const char * asset_path);
static uint_fast8_t     http_api_app_file_upload ();
static int8_t           http_decode_temp_correction (unsigned int value);
static unsigned int     http_encode_temp_correction (int temp_corr);
static int              http_clamp_temp_correction (int temp_corr);
static void             http_clear_request_user_agent (void);
static void             http_capture_request_user_agent (const String& line);
static const char *     http_get_request_browser_label (void);
static bool             http_read_request_line (String& line, unsigned long timeout_ms);

static const char * const APP_INSTALL_ASSETS[] =
{
    "app/index.html",
    "app/styles.css",
    "app/layout-previews.json",
    "app/icons/icon-192.svg",
    "app/icons/icon-512.svg",
    "app/icons/icon-192.png",
    "app/icons/icon-512.png",
    "app/icons/icon-180.png",
    "app/icons/icon-mask.png",
    "app/manifest.webmanifest",
    "app/app.js",
    /* Sprachdatei der PWA (B15). Pflichteintrag, und zwar aus drei Gruenden -- die
     * Weissliste ist NICHT der Tuersteher beim Ausliefern (http_app() fragt sie gar
     * nicht), sondern entscheidet an diesen Stellen:
     *   - http_api_app_file_upload() weist die Datei sonst mit error_code = 1 ab,
     *   - http_try_auto_install_app_files() holt sie bei der OTA-Nachinstallation nicht,
     *   - http_app_installation_complete() haelt die Installation faelschlich fuer
     *     vollstaendig.
     * Die Sprache bliebe stumm, obwohl im Repo alles stimmt -- derselbe Mechanismus,
     * der am 29.04.2026 die ganze PWA "verschwinden" liess (BEFUNDE.md L21). Daraus
     * folgt die Rollout-Reihenfolge: erst ESP flashen, dann die PWA hochladen.
     *
     * Im LittleFS heisst die Datei app-i18n-en.json.gz -- app_asset_filename()
     * ersetzt jeden '/' durch '-', es entsteht kein Unterordner (wie bei
     * app/icons/icon-192.png). Die Endung .json fuehrt http_app_asset_supports_gzip()
     * bereits, dort ist nichts zu tun.
     *
     * Jede weitere Sprache ist genau eine weitere Zeile hier -- aber zwei Dinge
     * bleiben gesetzt: (1) Der Vergleich in http_find_app_install_asset() bleibt
     * exakt, kein Praefixvergleich; er ist die einzige Pruefung vor einem
     * schreibenden LittleFS-Zugriff. (2) Der Basisname einer Sprachdatei ist der
     * Sprachcode und darf sonst nirgends im Asset-Baum vorkommen, weil
     * http_find_stored_app_asset_filename() hilfsweise auf den blossen Basisnamen
     * "en.json.gz" zurueckfaellt.
     */
    "app/i18n/en.json",
    "app/sw.js"
};
static int              http_api_live_display_color ();
static const char *     http_get_configured_icon_filename (void);
static const char *     http_get_configured_weather_filename (void);
static const char *     http_find_existing_filename (const char * preferred, const char * const * candidates, size_t candidate_count);
/* Maskierung ohne Haufen (BEFUNDE.md L175)
 *
 * sanitize_xml_string () und sanitize_json_string () bauten ihr Ergebnis ZEICHENWEISE
 * als String auf. Jedes Zeichen konnte eine Neuzuteilung ausloesen - String::changeBuffer ()
 * rundet auf 16-Byte-Schritte auf (WString.cpp), ein Wert von 63 Zeichen laeuft also
 * durch rund vier bis zwoelf realloc (). Das kostet kaum Gesamtspeicher, aber es
 * HINTERLAESST LUECKEN, und genau die sind das Problem: Gemessen ueber 25 Minuten
 * Betrieb faellt der groesste zusammenhaengende Block um 44 Prozent (5'800 auf 3'264),
 * waehrend die freie Gesamtmenge fast gleich bleibt. Die gescheiterten Anforderungen
 * aus L174 lauteten auf 880 und 960 Byte - in 5'800 passen die muehelos.
 *
 * Ersatz: ein Maskierer, der in einen Puffer des AUFRUFERS schreibt, und ein
 * Stroemer darueber, der den Text in Stuecken durch http_send () schiebt. Beide
 * fassen den Haufen nicht an. Die Ausgabe ist Byte fuer Byte dieselbe wie vorher.
 */
#define HTTP_ESCAPE_XML         0                       // wie sanitize_xml_string (): & < > " ' als Entity, ungueltiges UTF-8 als '?'
#define HTTP_ESCAPE_JSON        1                       // wie sanitize_json_string (): \\ " \r \n \t mit Rueckstrich maskiert
#define HTTP_ESCAPE_SCAN        2                       // XML-Entities INNERHALB von JSON, wie /api/network_scan es seit jeher liefert
#define HTTP_ESCAPE_JSON_LOG    3                       // C23: wie JSON, dazu \u00XX fuer Steuerzeichen und ISO-8859-1 nach UTF-8

static void             http_send_escaped (const char * s, uint_fast8_t mode);
static void             http_send_json_escaped (const char * s);
static void             http_send_xml_escaped (const char * s);
static void             update_progress_stream_emit (const char * event_name);
static void             update_progress_stream_start (void);
static void             update_progress_stream_finish (void);

typedef struct
{
    uint_fast8_t    active;
    char            type[16];
    char            state[24];
    char            message[128];
    uint32_t        progress_current;
    uint32_t        progress_total;
    uint32_t        error_code;
    uint32_t        started_at;
    uint32_t        updated_at;
    uint32_t        finished_at;
} UPDATE_PROGRESS;

static UPDATE_PROGRESS   update_progress;
static uint_fast8_t     update_progress_stream_active;
static char             http_request_user_agent[96];
static unsigned char    http_download_buf[1024];
static char             http_app_asset_local_filename[128];
static char             http_app_asset_remote_filename[128];
static char             http_app_asset_stale_filename[128];

/* Sec-Fetch-Dest des laufenden Requests.
 *
 * Browser senden diesen Header bei jeder Anfrage mit und nennen darin, WOFUER die
 * Antwort gedacht ist: "document" bei einem Seitenaufruf, "empty" bei fetch(),
 * "image" bei einem <img src=...>. Genau das unterscheidet einen bewussten Aufruf
 * von einem, den eine fremde Seite im Hintergrund ausloest.
 */
static char             http_request_fetch_dest[24];

static void
http_clear_request_fetch_dest (void)
{
    http_request_fetch_dest[0] = '\0';
}

static void
http_capture_request_fetch_dest (const String& line)
{
    String v;

    if (! line.startsWith ("Sec-Fetch-Dest:"))
    {
        return;
    }

    v = line.substring (15);
    v.trim ();
    strncpy (http_request_fetch_dest, v.c_str (), sizeof (http_request_fetch_dest) - 1);
    http_request_fetch_dest[sizeof (http_request_fetch_dest) - 1] = '\0';
}

/* Wahr, wenn die Antwort als eingebettete Ressource verwendet werden soll -- Bild,
 * Schrift, Stylesheet und dergleichen. Eine Wartungsaktion darf daraus nie entstehen.
 *
 * Ein <img src="http://<uhr>/api/maintenance_format_fs"> auf einer beliebigen Seite
 * im Heimnetz genuegte bisher, um das Dateisystem zu loeschen: die PWA, die
 * Layout-Tabelle und die Icon-Dateien. Die Rueckfragen dagegen stehen nur in der
 * Oberflaeche, nicht im Geraet -- wer die URL direkt aufruft, umgeht sie.
 *
 * Grenzen, bewusst in Kauf genommen: Aeltere Browser senden den Header nicht, dann
 * bleibt es beim bisherigen Verhalten. Gegen ein gezieltes curl hilft das ebenfalls
 * nicht. Es schliesst den Weg, der versehentlich getroffen wird, nicht jeden.
 */
static uint_fast8_t
http_request_is_embedded_subresource (void)
{
    static const char * const dests[] =
    {
        "image", "audio", "video", "font", "style", "script",
        "track", "embed", "object", "manifest", (const char *) 0
    };
    int i;

    if (! http_request_fetch_dest[0])
    {
        return 0;                                           // Header fehlt: nicht entscheidbar
    }

    for (i = 0; dests[i]; i++)
    {
        if (! strcmp (http_request_fetch_dest, dests[i]))
        {
            return 1;
        }
    }

    return 0;
}

static void
http_deny_embedded_subresource (const char * what)
{
    Serial.print ("- abgewiesen: ");
    Serial.print (what);
    Serial.print (" als Sec-Fetch-Dest=");
    Serial.println (http_request_fetch_dest);
    Serial.flush ();

    http_send (FS("HTTP/1.0 403 Forbidden\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":false,\"error\":\"embedded-subresource\"}"));
    http_flush ();
}

static void
http_clear_request_user_agent (void)
{
    http_request_user_agent[0] = '\0';
}

static void
http_capture_request_user_agent (const String& line)
{
    String ua;

    if (! line.startsWith ("User-Agent:"))
    {
        return;
    }

    ua = line.substring (11);
    ua.trim ();

    if (ua.length () == 0)
    {
        http_clear_request_user_agent ();
        return;
    }

    if (ua.indexOf ("FxiOS/") >= 0 || ua.indexOf ("Firefox/") >= 0)
    {
        strncpy (http_request_user_agent, "Firefox", sizeof (http_request_user_agent) - 1);
    }
    else if (ua.indexOf ("EdgiOS/") >= 0 || ua.indexOf ("EdgA/") >= 0 || ua.indexOf ("Edg/") >= 0)
    {
        strncpy (http_request_user_agent, "Edge", sizeof (http_request_user_agent) - 1);
    }
    else if (ua.indexOf ("OPiOS/") >= 0 || ua.indexOf ("OPR/") >= 0)
    {
        strncpy (http_request_user_agent, "Opera", sizeof (http_request_user_agent) - 1);
    }
    else if (ua.indexOf ("CriOS/") >= 0 || ua.indexOf ("Chrome/") >= 0)
    {
        strncpy (http_request_user_agent, "Chrome", sizeof (http_request_user_agent) - 1);
    }
    else if (ua.indexOf ("Safari/") >= 0)
    {
        strncpy (http_request_user_agent, "Safari", sizeof (http_request_user_agent) - 1);
    }
    else
    {
        strncpy (http_request_user_agent, ua.c_str (), sizeof (http_request_user_agent) - 1);
    }

    http_request_user_agent[sizeof (http_request_user_agent) - 1] = '\0';
}

static const char *
http_get_request_browser_label (void)
{
    return http_request_user_agent[0] ? http_request_user_agent : (const char *) 0;
}

static void
update_progress_copy (char * dst, size_t dst_size, const char * src)
{
    if (! dst || ! dst_size)
    {
        return;
    }

    strncpy (dst, src ? src : "", dst_size - 1);
    dst[dst_size - 1] = '\0';
}

static void
update_progress_stream_emit (const char * event_name)
{
    char buf[16];

    if (! update_progress_stream_active)
    {
        return;
    }

    http_send (FS("{\"ok\":true,\"event\":\""));
    http_send_json_escaped (event_name ? event_name : "progress");
    http_send (FS("\",\"active\":"));
    http_send (update_progress.active ? "true" : "false");
    http_send (FS(",\"type\":\""));
    http_send_json_escaped (update_progress.type);
    http_send (FS("\",\"state\":\""));
    http_send_json_escaped (update_progress.state);
    http_send (FS("\",\"message\":\""));
    http_send_json_escaped (update_progress.message);
    http_send (FS("\",\"progress_current\":"));
    sprintf (buf, "%u", (unsigned) update_progress.progress_current);
    http_send (buf);
    http_send (FS(",\"progress_total\":"));
    sprintf (buf, "%u", (unsigned) update_progress.progress_total);
    http_send (buf);
    http_send (FS(",\"error_code\":"));
    sprintf (buf, "%u", (unsigned) update_progress.error_code);
    http_send (buf);
    http_send (FS(",\"started_at\":"));
    sprintf (buf, "%u", (unsigned) update_progress.started_at);
    http_send (buf);
    http_send (FS(",\"updated_at\":"));
    sprintf (buf, "%u", (unsigned) update_progress.updated_at);
    http_send (buf);
    http_send (FS(",\"finished_at\":"));
    sprintf (buf, "%u", (unsigned) update_progress.finished_at);
    http_send (buf);
    http_send (FS("}\n"));
    http_flush ();
}

static void
update_progress_stream_start (void)
{
    update_progress_stream_active = 1;
}

static void
update_progress_stream_finish (void)
{
    update_progress_stream_active = 0;
}

static uint_fast8_t
http_filename_matches (const char * actual, const char * expected)
{
    return actual && expected && ! strcmp (actual, expected) ? 1 : 0;
}

static uint_fast8_t
http_tables_filename_matches (const char * actual, const char * target_filename)
{
    size_t expected_prefix_len;

    if (! actual || ! target_filename)
    {
        return 0;
    }

    expected_prefix_len = strlen (target_filename);

    if (expected_prefix_len < strlen ("local.txt"))
    {
        return 0;
    }

    expected_prefix_len -= strlen ("local.txt");

    if (strncmp (actual, target_filename, expected_prefix_len))
    {
        return 0;
    }

    return strstr (actual, ".txt") && ! strcmp (actual + strlen (actual) - 4, ".txt") ? 1 : 0;
}

static const char *
http_get_configured_icon_filename (void)
{
    if (hardware_configuration == 0xFFFF)
    {
        return (const char *) 0;
    }

    switch (hardware_configuration & HW_WC_MASK)
    {
        case HW_WC_24H: return "wc24h-icon.txt";
        case HW_WC_12H: return "wc12h-icon.txt";
        case HW_UCLOCK: return "uc-icon.txt";
    }

    return (const char *) 0;
}

static const char *
http_get_configured_weather_filename (void)
{
    if (hardware_configuration == 0xFFFF)
    {
        return (const char *) 0;
    }

    switch (hardware_configuration & HW_WC_MASK)
    {
        case HW_WC_24H: return "wc24h-weather.txt";
        case HW_WC_12H: return "wc12h-weather.txt";
        case HW_UCLOCK: return "uc-weather.txt";
    }

    return (const char *) 0;
}

static const char *
http_get_tables_family_prefix (void)
{
    if (hardware_configuration == 0xFFFF)
    {
        return (const char *) 0;
    }

    switch (hardware_configuration & HW_WC_MASK)
    {
        case HW_WC_24H: return "wc24h-tables-";
        case HW_WC_12H: return "wc12h-tables-";
    }

    return (const char *) 0;
}

static const char *
http_get_default_layout_preview_filename (void)
{
    if (hardware_configuration == 0xFFFF)
    {
        return (const char *) 0;
    }

    switch (hardware_configuration & HW_WC_MASK)
    {
        case HW_WC_24H: return "wc24h-tables-de.txt";
        case HW_WC_12H: return "wc12h-tables-de.txt";
    }

    return (const char *) 0;
}

static uint_fast8_t
http_get_default_layout_columns (void)
{
    if (hardware_configuration == 0xFFFF)
    {
        return 11;
    }

    switch (hardware_configuration & HW_WC_MASK)
    {
        case HW_WC_24H: return 18;
        case HW_WC_12H: return 11;
        default: return 11;
    }
}

static bool
http_read_request_line (String& line, unsigned long timeout_ms)
{
    int             c = -1;
    unsigned long   start_millis = millis ();

    line = "";

    do
    {
        if (http_client.available ())
        {
            start_millis = millis ();
            c = http_client.read ();

            if (c >= 0 && c != '\n' && c != '\r')
            {
                line += (char) c;
            }
        }
        else if ((millis () - start_millis) >= timeout_ms)
        {
            return false;
        }
    } while (c >= 0 && c != '\r');

    return true;
}

static const char *
http_get_asset_prefix (void)
{
    if (hardware_configuration == 0xFFFF)
    {
        return (const char *) 0;
    }

    switch (hardware_configuration & HW_WC_MASK)
    {
        case HW_WC_24H: return "wc24h";
        case HW_WC_12H: return "wc12h";
        case HW_UCLOCK: return "uc";
    }

    return (const char *) 0;
}

static const char *
http_get_hardware_label (void)
{
    switch (hardware_configuration & HW_WC_MASK)
    {
        case HW_WC_24H: return "WC24h";
        case HW_WC_12H: return "WC12h";
        case HW_UCLOCK: return "uClock";
    }

    return "unbekannt";
}

static const char *
http_get_processor_label (void)
{
    switch (hardware_configuration & HW_STM32_MASK)
    {
        case HW_STM32_F103C8: return "STM32F103C8";
        case HW_STM32_F401RE: return "STM32F401RE";
        case HW_STM32_F411RE: return "STM32F411RE";
        case HW_STM32_F446RE: return "STM32F446RE";
        case HW_STM32_F407VE: return "STM32F407VE";
        case HW_STM32_F401CC: return "STM32F401CC";
        case HW_STM32_F411CE: return "STM32F411CE";
    }

    return "unbekannt";
}

static const char *
http_get_board_label (void)
{
    switch (hardware_configuration & HW_STM32_MASK)
    {
        case HW_STM32_F103C8: return "BluePill";
        case HW_STM32_F401RE:
        case HW_STM32_F411RE:
        case HW_STM32_F446RE: return "Nucleo";
        case HW_STM32_F407VE: return "BlackBoard";
        case HW_STM32_F401CC:
        case HW_STM32_F411CE: return "BlackPill";
    }

    return "unbekannt";
}

static const char *
http_get_frequency_label (void)
{
    switch (hardware_configuration & HW_STM32_MASK)
    {
        case HW_STM32_F103C8: return "72 MHz";
        case HW_STM32_F401RE: return "84 MHz";
        case HW_STM32_F411RE: return "100 MHz";
        case HW_STM32_F446RE: return "180 MHz";
        case HW_STM32_F407VE: return "168 MHz";
        case HW_STM32_F401CC: return "84 MHz";
        case HW_STM32_F411CE: return "100 MHz";
    }

    return "unbekannt";
}

static const char *
http_get_oscillator_label (void)
{
    switch (hardware_configuration & HW_OSC_FREQUENCY_MASK)
    {
        case HW_OSC_FREQUENCY_8MHZ: return "8 MHz";
        case HW_OSC_FREQUENCY_25MHZ: return "25 MHz";
    }

    return "unbekannt";
}

static const char *
http_get_display_label (void)
{
    switch (hardware_configuration & HW_LED_MASK)
    {
        case HW_LED_WS2812_GRB_LED: return "WS2812 GRB";
        case HW_LED_WS2812_RGB_LED: return "WS2812 RGB";
        case HW_LED_APA102_RGB_LED: return "APA102 RGB";
        case HW_LED_SK6812_RGB_LED: return "SK6812 RGB";
        case HW_LED_SK6812_RGBW_LED: return "SK6812 RGBW";
        case HW_LED_TFTLED_RGB_LED: return "TFT RGB";
    }

    return "unbekannt";
}

static const char *
http_get_display_led_mode (void)
{
    switch (hardware_configuration & HW_LED_MASK)
    {
        case HW_LED_SK6812_RGBW_LED: return "rgbw";
        case HW_LED_WS2812_GRB_LED:
        case HW_LED_WS2812_RGB_LED:
        case HW_LED_APA102_RGB_LED:
        case HW_LED_SK6812_RGB_LED: return "rgb";
        case HW_LED_TFTLED_RGB_LED: return "tft";
    }

    return "none";
}

static void
http_json_send_field_prefix (const char * key)
{
    http_send (FS(",\""));
    http_send (key);
    http_send (FS("\":"));
}

static void
http_json_send_bool_field (const char * key, uint_fast8_t value)
{
    http_json_send_field_prefix (key);
    http_send (value ? "true" : "false");
}

static void
http_json_send_uint_field (const char * key, unsigned long value)
{
    http_json_send_field_prefix (key);
    http_send (String (value).c_str ());
}

static void
http_json_send_string_field (const char * key, const char * value)
{
    http_json_send_field_prefix (key);
    http_send (FS("\""));
    http_send_json_escaped (value ? value : "");
    http_send (FS("\""));
}

static const char *
http_get_local_update_message (uint32_t flashsize)
{
    if (flashsize < 1048576UL)
    {
        return "Lokales Update ist bei dieser ESP-Flashgroesse nicht verfuegbar.";
    }

    return "ESP- oder STM32-Datei auswaehlen und direkt lokal hochladen.";
}

static const char *
http_find_existing_filename (const char * preferred, const char * const * candidates, size_t candidate_count)
{
    size_t idx;

    if (preferred && LittleFS.exists (preferred))
    {
        return preferred;
    }

    for (idx = 0; idx < candidate_count; idx++)
    {
        const char * candidate = candidates[idx];

        if (candidate && (! preferred || strcmp (candidate, preferred)) && LittleFS.exists (candidate))
        {
            return candidate;
        }
    }

    return preferred;
}

static uint_fast8_t
http_local_stm32_filename_matches (const char * actual)
{
    char            default_filename[MAX_UPDATE_FILENAME_LEN];
    const char *    filter = (const char *) NULL;

    if (! actual)
    {
        return 0;
    }

    http_build_stm32_default_filename (default_filename, sizeof (default_filename), &filter);

    if (! default_filename[0])
    {
        return 0;
    }

    return http_filename_matches (actual, default_filename);
}

static uint_fast8_t
http_remote_stm32_filename_matches (const char * actual)
{
    char            default_filename[MAX_UPDATE_FILENAME_LEN];
    const char *    filter = (const char *) NULL;
    const char *    p;
    size_t          prefix_len;
    size_t          filter_len;
    size_t          actual_len;

    if (! actual || ! *actual)
    {
        return 0;
    }

    actual_len = strlen (actual);

    http_build_stm32_default_filename (default_filename, sizeof (default_filename), &filter);

    if (! filter)
    {
        return 0;
    }

    filter_len = strlen (filter);

    /* Bisherige Regel, unveraendert: sechs Zeichen Layoutpraefix (wc24h-, wc12h-), dahinter der
     * Chip. Sie laesst den Wechsel zwischen wc12h und wc24h zu und bleibt deshalb stehen - jeder
     * Name, der hier bisher passte, passt weiter (Review F.6, A2).
     */
    if (actual_len >= 6 && ! strncmp (actual + 6, filter, filter_len))
    {
        return 1;
    }

    /* A2: Das Praefix ist nicht immer sechs Zeichen lang - "uc-" (HW_UCLOCK) hat drei, und ein
     * uc-Name passte damit nie, auch der eigene Standardname nicht. Zusaetzlich gilt deshalb: Der
     * Name beginnt mit dem Praefix DIESES Geraets, und direkt dahinter steht der Chip. Das Praefix
     * steht im Standardnamen vor dem Filter; http_build_stm32_default_filename () setzt beide so
     * zusammen. Ein leeres Praefix (unbekannte Layoutkennung) laesst nichts zusaetzlich durch.
     */
    p          = strstr (default_filename, filter);
    prefix_len = p ? (size_t) (p - default_filename) : 0;

    if (prefix_len > 0 && actual_len >= prefix_len + filter_len
        && ! strncmp (actual, default_filename, prefix_len) && ! strncmp (actual + prefix_len, filter, filter_len))
    {
        return 1;
    }

    return 0;
}

static uint_fast8_t
http_local_esp_filename_matches (const char * actual)
{
    size_t len;

    if (! actual)
    {
        return 0;
    }

    len = strlen (actual);

    return len >= 4 && ! strcmp (actual + len - 4, ".bin") ? 1 : 0;
}

static uint_fast8_t
http_table_download_filename_matches (const char * actual)
{
    const char * prefix = (const char *) NULL;

    if (! actual)
    {
        return 0;
    }

    switch (hardware_configuration & HW_WC_MASK)
    {
        case HW_WC_24H: prefix = "wc24h-tables-"; break;
        case HW_WC_12H: prefix = "wc12h-tables-"; break;
        default:        prefix = (const char *) NULL; break;
    }

    if (! prefix)
    {
        return 0;
    }

    return ! strncmp (actual, prefix, strlen (prefix)) && strstr (actual, ".txt") && ! strcmp (actual + strlen (actual) - 4, ".txt") ? 1 : 0;
}

static void
http_remove_table_family_files (const char * keep_filename)
{
    Dir             dir;
    const char *    prefix = (const char *) NULL;

    switch (hardware_configuration & HW_WC_MASK)
    {
        case HW_WC_24H: prefix = "wc24h-tables-"; break;
        case HW_WC_12H: prefix = "wc12h-tables-"; break;
        default:        prefix = (const char *) NULL; break;
    }

    if (! prefix)
    {
        return;
    }

    dir = LittleFS.openDir ("");

    while (dir.next ())
    {
        String str = dir.fileName ();
        const char * current = str.c_str ();
        int len = strlen (current);

        if (! strncmp (current, prefix, strlen (prefix)) &&
            len > 4 &&
            ! strcmp (current + len - 4, ".txt") &&
            (! keep_filename || strcmp (current, keep_filename)))
        {
            LittleFS.remove (current);
        }
    }
}

void
update_progress_begin (const char * type, const char * state, const char * message)
{
    memset (&update_progress, 0, sizeof (update_progress));
    update_progress.active = 1;
    update_progress.started_at = millis ();
    update_progress.updated_at = update_progress.started_at;
    update_progress_copy (update_progress.type, sizeof (update_progress.type), type);
    update_progress_copy (update_progress.state, sizeof (update_progress.state), state ? state : "starting");
    update_progress_copy (update_progress.message, sizeof (update_progress.message), message);
    update_progress_stream_emit ("begin");
}

void
update_progress_state (const char * state, const char * message)
{
    update_progress.updated_at = millis ();

    if (state)
    {
        update_progress_copy (update_progress.state, sizeof (update_progress.state), state);
    }

    if (message)
    {
        update_progress_copy (update_progress.message, sizeof (update_progress.message), message);
    }

    update_progress_stream_emit ("state");
}

void
update_progress_set_progress (uint32_t current, uint32_t total)
{
    update_progress.progress_current = current;
    update_progress.progress_total = total;
    update_progress.updated_at = millis ();
    update_progress_stream_emit ("progress");
}

void
update_progress_complete (const char * message)
{
    update_progress.active = 0;
    update_progress.updated_at = millis ();
    update_progress.finished_at = update_progress.updated_at;
    update_progress_copy (update_progress.state, sizeof (update_progress.state), "done");

    if (message)
    {
        update_progress_copy (update_progress.message, sizeof (update_progress.message), message);
    }

    update_progress_stream_emit ("complete");
}

void
update_progress_fail (uint32_t error_code, const char * message)
{
    update_progress.active = 0;
    update_progress.error_code = error_code;
    update_progress.updated_at = millis ();
    update_progress.finished_at = update_progress.updated_at;
    update_progress_copy (update_progress.state, sizeof (update_progress.state), "error");

    if (message)
    {
        update_progress_copy (update_progress.message, sizeof (update_progress.message), message);
    }

    update_progress_stream_emit ("error");
}

void
update_progress_clear (void)
{
    memset (&update_progress, 0, sizeof (update_progress));
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * flush output buffer
 *
 * WiFiClient::write () liefert die Zahl der tatsaechlich uebernommenen Byte, und die
 * darf kleiner sein als die uebergebene Laenge: ClientContext::_write_from_source ()
 * bricht ab, sobald _timeout_ms (Vorgabe 5000 ms) ohne jeden Fortschritt verstrichen
 * ist, und gibt zurueck, was bis dahin durchkam (Core 3.1.2, ClientContext.h:463-494).
 * Ohne Fortschritt bleibt es, wenn tcp_sndbuf () leer bleibt oder tcp_write () mit
 * ERR_MEM antwortet. Letzteres ist hier real: _sync ist aus, also kopiert lwIP jeden
 * Block in frisch angeforderte pbufs - bei rund 5'400 Byte freiem Heap und 4'648 Byte
 * groesstem zusammenhaengendem Block (L134) ist das keine theoretische Lage.
 *
 * Bis ESP 3.2.14 stand hier http_client.print () ohne Auswertung des Rueckgabewerts.
 * Das ergab den lautlosesten denkbaren Fehler: Genau ein Pufferinhalt fehlt mitten in
 * der Antwort, alles danach geht korrekt hinaus, und weil diese Antworten ohne
 * Content-Length mit "Connection: close" laufen, sieht der Client ein sauberes Ende.
 * Bei /api/update_status blieb das Ergebnis sogar gueltiges JSON - 8084 Byte mit 162
 * Feldern gegen 7012 Byte mit 136 Feldern, ohne eine Fehlermeldung irgendwo.
 *
 * Hoechstens ein Wiederholungsversuch, und nur wenn der erste Aufruf ueberhaupt etwas
 * geschrieben hat: Ein Aufruf, der gar nichts unterbringt, hat bereits fuenf Sekunden
 * auf Fortschritt gewartet - ein zweiter wartet erneut so lange und findet dieselbe
 * Lage vor. So bleibt die Dauer eines Fehlschlags bei den heutigen rund 5 s.
 *
 * Das frueher hier stehende http_client.flush () ist entfallen. Es ist im Core 3.1.2
 * kein Leeren, sondern ein Warten bis zur Quittung mit bis zu 300 ms
 * (WIFICLIENT_MAX_FLUSH_WAIT_MS, L136) - und zwar je Block, also achtmal in einer
 * 8-KB-Antwort. Noetig ist es nicht: write () kehrt erst zurueck, wenn lwIP die Daten
 * uebernommen hat, WiFiClient::stop () wartet ohnehin selbst bis zur Quittung
 * (WiFiClient.cpp:316-325), und vor dem stop () am Ende von http_server_loop () steht
 * weiterhin ein flush (). Auch fuer die Wiederverwendung von http_response als
 * Lesepuffer in http_send_fs_file () aendert sich nichts: _sync ist aus
 * (WiFiClient.cpp:46), der Core setzt deshalb TCP_WRITE_FLAG_COPY und lwIP haelt eine
 * eigene Kopie - der Puffer darf unmittelbar nach write () neu befuellt werden.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
http_flush (void)
{
    if (http_response_len > 0)
    {
        size_t rest = (size_t) http_response_len;

        if (! http_write_broken)
        {
            const uint8_t * p = (const uint8_t *) http_response;
            uint_fast8_t    attempt;

            for (attempt = 0; attempt < 2; attempt++)
            {
                size_t written = http_client.write (p, rest);

                if (written >= rest)
                {
                    rest = 0;
                    break;
                }

                p    += written;
                rest -= written;

                if (written == 0 || ! http_client.connected ())
                {
                    break;
                }

                yield ();
            }
        }

        if (rest > 0)
        {
            http_write_lost_bytes += (uint32_t) rest;

            if (http_write_lost_blocks < 0xFFFF)
            {
                http_write_lost_blocks++;
            }

            if (! http_client.connected ())                     // C25: Browser-Abbau und Schreibversagen getrennt zaehlen
            {
                if (http_write_lost_gone < 0xFFFF)
                {
                    http_write_lost_gone++;
                }
            }
            else if (http_write_lost_failed < 0xFFFF)
            {
                http_write_lost_failed++;
            }

            if (! http_write_broken)
            {
                /* Genau eine Zeile je Verbindung. Jede Zeile hier geht auf die
                 * STM-UART, deren RX-Ring 256 Byte fasst und bei Ueberlauf still
                 * verwirft (uart-driver.h:698). Die Folgebloecke derselben Antwort
                 * laufen ueber den Zweig oben und schweigen. Fuer den Logring gilt
                 * dasselbe: Das stm32_log_append () unten steht INNERHALB dieses
                 * Zweiges, nicht daneben - sonst waere es eine Zeile je Block.
                 *
                 * EINMAL formatieren, ZWEIMAL ausgeben: Mitschnitt und API duerfen
                 * nicht auseinanderlaufen. snprintf in einen Stackpuffer, KEIN
                 * String - dessen Aufbau ist nach L175 der benannte Treiber der
                 * Fragmentierung. Laengste Form: "- http write lost 4294967295
                 * gone=65535 fail=65535" = 50 Zeichen (C25: beide Zaehler, Stand nach
                 * diesem Verlust - welcher gestiegen ist, nennt den Grund).
                 *
                 * Ohne das stm32_log_append () war diese Zeile nur mit
                 * angeschlossenem Mitschnitt zu sehen; ueber /api/stm32_log kam
                 * nichts an (C14, L185).
                 */
                char line[64];

                http_write_broken = 1;

                snprintf (line, sizeof (line), "- http write lost %lu gone=%u fail=%u",
                          (unsigned long) rest, (unsigned) http_write_lost_gone, (unsigned) http_write_lost_failed);

                Serial.println (line);
                Serial.flush ();
                stm32_log_append (line);
            }
        }

        http_response[0] = '\0';
        http_response_len = 0;
    }
}

static void
http_concat_response (const char * s, int len)
{
    strncpy (http_response + http_response_len, s, len);
    http_response_len += len;
    http_response[http_response_len] = '\0';
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * send string
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
http_send (const char * s)
{
    int len = strlen (s);

    while (http_response_len + len > MAX_HTTP_RESPONSE_LEN)
    {
        int rest = MAX_HTTP_RESPONSE_LEN - http_response_len;

        http_concat_response (s, rest);
        http_flush ();

        len -= rest;
        s += rest;
    }

    http_concat_response (s, len);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * send string
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
http_send (String s)
{
    http_send (s.c_str());
}

/* Ein Zeichen je http_send () war nicht nur langsam, sondern legte fuer jede
 * Maskierung ueber FS() ein String-Objekt an. Jetzt laeuft alles ueber denselben
 * Stroemer wie die XML-Seite; die Ausgabe ist unveraendert.
 */
static void
http_send_json_escaped (const char * s)
{
    http_send_escaped (s, HTTP_ESCAPE_JSON);
}

static const char *
http_content_type (const char * path)
{
    const char * p = strrchr (path, '.');

    if (! p)
    {
        return "text/plain";
    }

    if (! strcmp (p, ".html"))
    {
        return "text/html; charset=utf-8";
    }
    else if (! strcmp (p, ".css"))
    {
        return "text/css; charset=utf-8";
    }
    else if (! strcmp (p, ".js"))
    {
        return "application/javascript; charset=utf-8";
    }
    else if (! strcmp (p, ".webmanifest"))
    {
        return "application/manifest+json; charset=utf-8";
    }
    else if (! strcmp (p, ".json"))
    {
        return "application/json; charset=utf-8";
    }
    else if (! strcmp (p, ".svg"))
    {
        return "image/svg+xml";
    }
    else if (! strcmp (p, ".png"))
    {
        return "image/png";
    }
    else if (! strcmp (p, ".ico"))
    {
        return "image/x-icon";
    }

    return "text/plain";
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * http_send_fs_file () - eine Datei aus dem LittleFS ausliefern
 *
 * Rueckgabewert:
 *   0 = nichts gesendet, der Aufrufer darf eine eigene Antwort schreiben
 *   1 = vollstaendig gesendet
 *   2 = nach gesendeten Kopfzeilen abgebrochen, Verbindung verbraucht -
 *       der Aufrufer darf KEINE zweite Antwort mehr in diese Verbindung schreiben
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_send_fs_file (const char * filename, const char * content_type, uint_fast8_t gzip_encoded)
{
    uint_fast8_t    rtc = 0;

    LittleFS.begin ();

    /* Nicht LittleFS.exists(): Eine 0-Byte-Datei existiert und wuerde mit 200 OK
     * ausgeliefert. Bei app.js.gz bedeutet das einen weissen Bildschirm, und die PWA
     * ist danach nicht mehr bedienbar, um es zu korrigieren. Genau dieser Fall ist
     * real eingetreten; die Invariante steht in CLAUDE.md.
     */
    if (http_fs_file_exists_and_nonempty (filename))
    {
        File fp = LittleFS.open (filename, "r");

        if (fp)
        {
            /* Nagle nur fuer die Dateiauslieferung einschalten. Die pauschale Zeile
             * http_client.setNoDelay (1) beim Annehmen der Verbindung bleibt stehen:
             * fuer die kleinen API-Antworten, die die PWA pollt, ist "sofort raus"
             * richtig. Hier dagegen verhindert sie das Verschmelzen zu vollen
             * MSS-Segmenten - am Geraet gemessen gingen 129'477 Byte in 758 Paketen
             * hinaus statt in 242 (L131), also 1,50 Pakete je 256-Byte-Block.
             * Zuruecksetzen ist nicht noetig: Die Verbindung wird unten mit stop()
             * geschlossen, und jede neue Verbindung setzt den Wert wieder auf 1.
             */
            http_client.setNoDelay (0);

            http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: "));
            http_send (content_type);

            if (gzip_encoded)
            {
                http_send (FS("\r\nContent-Encoding: gzip"));
            }

            http_send (FS("\r\nContent-Length: "));
            http_send (String (fp.size ()).c_str ());
            http_send (FS("\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n"));
            http_flush ();

            /* Lesepuffer ist bewusst der bereits vorhandene statische Antwortpuffer:
             * 2 x TCP_MSS = 1072 Byte (MAX_HTTP_RESPONSE_LEN, Begruendung dort), unmittelbar
             * darueber mit http_flush() geleert und bis zum Verbindungsende ungenutzt. Die
             * Blockgroesse ist damit ein Vielfaches der Segmentgroesse - jeder Schreibvorgang
             * fuellt genau zwei Segmente. Ein eigener Puffer auf dem
             * Stack kostete 768 Byte vom 4-KB-cont-Stack, ein eigener statischer Puffer
             * 1 KB vom ohnehin knappen Heap - bei einem Speicherbefund beides die
             * falsche Richtung.
             * BEDINGUNG: Zwischen dem http_flush() oben und dem Ende der Schleife darf
             * kein http_send() stehen, sonst ueberschreiben sich beide Nutzungen.
             */
            while (fp.available ())
            {
                int len = fp.read ((uint8_t *) http_response, MAX_HTTP_RESPONSE_LEN);

                /* Vorzeichenbehafteter Typ mit Absicht: File::read() liefert int.
                 * Der Core 3.1.2 gibt im Fehlerfall 0 zurueck (LittleFS.h:411-422,
                 * nachgesehen); mit size_t wuerde aus einem spaeteren -1 ein
                 * Schreibvorgang ueber 4 GB. Ohne break liefe die Schleife hier
                 * endlos, weil die Dateiposition nicht vorrueckt (L129 a).
                 */
                if (len <= 0)
                {
                    Serial.print ("- fs read error: ");
                    Serial.println (filename);
                    Serial.flush ();
                    rtc = 2;
                    break;
                }

                /* Kurzschreibung: WiFiClient::write () liefert die tatsaechlich
                 * uebergebene Menge und bricht nach seinem Zeitlimit oder bei
                 * geschlossener Verbindung frueher ab. Ungeprueft entstuende eine
                 * stillschweigend verstuemmelte Datei beim Client.
                 */
                if (http_client.write ((const uint8_t *) http_response, (size_t) len) != (size_t) len)
                {
                    Serial.print ("- fs write short: ");
                    Serial.println (filename);
                    Serial.flush ();
                    rtc = 2;
                    break;
                }

                yield ();
            }

            http_client.flush ();
            fp.close ();
            http_client.stop ();

            if (rtc == 0)
            {
                rtc = 1;
            }
        }
    }

    LittleFS.end ();

    return rtc;
}

static bool
http_fs_file_exists_and_nonempty (const char * filename)
{
    File fp;
    bool rtc = false;

    if (LittleFS.exists (filename))
    {
        fp = LittleFS.open (filename, "r");

        if (fp)
        {
            rtc = (fp.size () > 0);
            fp.close ();
        }
    }

    return rtc;
}

static bool
http_app_installation_complete (void)
{
    size_t  asset_count = sizeof (APP_INSTALL_ASSETS) / sizeof (APP_INSTALL_ASSETS[0]);
    bool    rtc = true;

    LittleFS.begin ();

    for (size_t idx = 0; idx < asset_count; idx++)
    {
        const char * asset_path = APP_INSTALL_ASSETS[idx];
        char flat_gz[72];
        char native_gz[72];

        const char * slash = strrchr (asset_path, '/');
        const char * base  = slash ? slash + 1 : asset_path;

        app_asset_storage_filename (asset_path, 1, flat_gz, sizeof (flat_gz));
        snprintf (native_gz, sizeof (native_gz), "%s.gz", base);

        if (! http_fs_file_exists_and_nonempty (flat_gz) &&
            ! http_fs_file_exists_and_nonempty (native_gz))
        {
            rtc = false;
            break;
        }
    }

    LittleFS.end ();
    return rtc;
}

static bool         download_file (const char * host, const char * path, const char * filename);
static bool         download_file_to_local (const char * host, const char * path, const char * remote_filename, const char * local_filename);
static bool         http_remote_app_files_available (char * version_buf, size_t version_buf_len);
static bool         download_file_as_flattened_app_asset (const char * host, const char * path, const char * remote_filename);

static bool
download_file_as_flattened_app_asset (const char * host, const char * path, const char * remote_filename)
{
    const char *    requested_remote_filename = remote_filename;
    uint_fast8_t    gzip_encoded = 0;

    if (http_app_asset_supports_gzip (remote_filename))
    {
        snprintf (http_app_asset_remote_filename, sizeof (http_app_asset_remote_filename), "%s.gz", remote_filename);
        requested_remote_filename = http_app_asset_remote_filename;
        gzip_encoded = 1;
    }

    if (! app_asset_storage_filename (remote_filename, gzip_encoded, http_app_asset_local_filename, sizeof (http_app_asset_local_filename)))
    {
        return false;
    }

    if (! download_file_to_local (host, path, requested_remote_filename, http_app_asset_local_filename))
    {
        return false;
    }

    if (app_asset_storage_filename (remote_filename, gzip_encoded ? 0 : 1, http_app_asset_stale_filename, sizeof (http_app_asset_stale_filename)))
    {
        LittleFS.remove (http_app_asset_stale_filename);
    }

    return true;
}

static int
http_try_auto_install_app_files (void)
{
    STR_VAR *   sv;
    char *      update_host;
    char *      update_path;
    size_t      idx;
    size_t      asset_count = sizeof (APP_INSTALL_ASSETS) / sizeof (APP_INSTALL_ASSETS[0]);
    int         rtc = 1;

    sv = get_strvar (UPDATE_HOST_VAR);
    update_host = sv->str;

    if (! update_host[0])
    {
        update_host = (char *) DEFAULT_UPDATE_HOST;
    }

    sv = get_strvar (UPDATE_PATH_VAR);
    update_path = sv->str;

    if (! update_path[0])
    {
        update_path = (char *) DEFAULT_UPDATE_PATH;
    }

    LittleFS.begin ();
    Serial.print (FS("(APPDL app-install-begin count="));
    Serial.print (asset_count);
    Serial.println (FS(")"));

    for (idx = 0; idx < asset_count; idx++)
    {
        uint_fast8_t is_gz = http_app_asset_supports_gzip (APP_INSTALL_ASSETS[idx]);

        if (! app_asset_storage_filename (APP_INSTALL_ASSETS[idx], is_gz, http_app_asset_local_filename, sizeof (http_app_asset_local_filename)))
        {
            Serial.print (FS("(APPDL app-install-fail step="));
            Serial.print (idx + 1);
            Serial.print (FS("/"));
            Serial.print (asset_count);
            Serial.print (FS(" remote="));
            Serial.print (APP_INSTALL_ASSETS[idx]);
            Serial.println (FS(" reason=filename)"));
            rtc = 0;
            break;
        }

        Serial.print (FS("(APPDL app-install-step "));
        Serial.print (idx + 1);
        Serial.print (FS("/"));
        Serial.print (asset_count);
        Serial.print (FS(" remote="));
        Serial.print (APP_INSTALL_ASSETS[idx]);
        if (is_gz) { Serial.print (FS(".gz")); }
        Serial.print (FS(" local="));
        Serial.print (http_app_asset_local_filename);
        Serial.println (FS(")"));

        if (! download_file_as_flattened_app_asset (update_host, update_path, APP_INSTALL_ASSETS[idx]))
        {
            Serial.print (FS("(APPDL app-install-fail step="));
            Serial.print (idx + 1);
            Serial.print (FS("/"));
            Serial.print (asset_count);
            Serial.print (FS(" remote="));
            Serial.print (APP_INSTALL_ASSETS[idx]);
            Serial.print (FS(" local="));
            Serial.print (http_app_asset_local_filename);
            Serial.println (FS(" reason=download)"));
            rtc = 0;
            break;
        }
    }

    LittleFS.end ();

    if (rtc)
    {
        Serial.print (FS("(APPDL app-install-complete count="));
        Serial.print (asset_count);
        Serial.println (FS(")"));
    }

    return rtc;
}

static bool
http_remote_app_files_available (char * version_buf, size_t version_buf_len)
{
    STR_VAR *   sv;
    char *      update_host;
    char *      update_path;
    char        remote_filename[128];
    bool        app_files_available = false;

    if (version_buf && version_buf_len)
    {
        version_buf[0] = '\0';
    }

    sv = get_strvar (UPDATE_HOST_VAR);
    update_host = sv->str;

    if (! update_host[0])
    {
        update_host = (char *) DEFAULT_UPDATE_HOST;
    }

    sv = get_strvar (UPDATE_PATH_VAR);
    update_path = sv->str;

    if (! update_path[0])
    {
        update_path = (char *) DEFAULT_UPDATE_PATH;
    }

    if (version_buf && version_buf_len)
    {
        http_fetch_remote_line (update_host, update_path, APP_VERSION_TXT, version_buf, version_buf_len);
    }

    if (http_app_asset_supports_gzip (APP_INSTALL_ASSETS[0]))
    {
        snprintf (remote_filename, sizeof (remote_filename), "%s.gz", APP_INSTALL_ASSETS[0]);

        if (httpclient (update_host, update_path, remote_filename) > 0)
        {
            app_files_available = true;
            httpclient_stop ();
        }
    }
    else if (httpclient (update_host, update_path, APP_INSTALL_ASSETS[0]) > 0)
    {
        app_files_available = true;
        httpclient_stop ();
    }

    return app_files_available;
}

static bool
app_asset_filename (const char * asset_path, char * filename, size_t maxlen)
{
    size_t idx = 0;

    if (! asset_path || ! *asset_path)
    {
        return false;
    }

    for (const char * p = asset_path; *p && idx < maxlen - 1; p++)
    {
        if (*p == '/')
        {
            filename[idx++] = '-';
        }
        else
        {
            filename[idx++] = *p;
        }
    }

    filename[idx] = '\0';
    return idx > 0;
}

static const char *
http_find_app_install_asset (const char * asset_path)
{
    size_t asset_count = sizeof (APP_INSTALL_ASSETS) / sizeof (APP_INSTALL_ASSETS[0]);

    if (! asset_path || ! *asset_path)
    {
        return (const char *) 0;
    }

    for (size_t idx = 0; idx < asset_count; idx++)
    {
        if (! strcmp (asset_path, APP_INSTALL_ASSETS[idx]))
        {
            return APP_INSTALL_ASSETS[idx];
        }
    }

    return (const char *) 0;
}

static uint_fast8_t
http_app_asset_supports_gzip (const char * asset_path)
{
    const char * p = strrchr (asset_path ? asset_path : "", '.');

    if (! p)
    {
        return 0;
    }

    return (! strcmp (p, ".html") ||
            ! strcmp (p, ".css") ||
            ! strcmp (p, ".js") ||
            ! strcmp (p, ".json") ||
            ! strcmp (p, ".webmanifest") ||
            ! strcmp (p, ".svg") ||
            // PNG ist bereits komprimiert, gzip bringt hier unter einem Prozent.
            // Es steht trotzdem hier, weil http_find_stored_app_asset_filename
            // AUSSCHLIESSLICH nach .gz sucht: eine Datei ohne diese Endung wuerde
            // im LittleFS liegen und nie gefunden.
            ! strcmp (p, ".png"));
}

static bool
app_asset_storage_filename (const char * asset_path, uint_fast8_t gzip_encoded, char * filename, size_t maxlen)
{
    size_t len;

    if (! app_asset_filename (asset_path, filename, maxlen))
    {
        return false;
    }

    if (! gzip_encoded)
    {
        return true;
    }

    len = strlen (filename);

    if (len + 3 >= maxlen)
    {
        return false;
    }

    strcpy (filename + len, ".gz");
    return true;
}

static uint_fast8_t
http_find_stored_app_asset_filename (const char * asset_path, char * filename, size_t maxlen, uint_fast8_t * gzip_encoded)
{
    char            local_filename[128];
    uint_fast8_t    rtc = 0;

    LittleFS.begin ();

    // flattened .gz  (OTA / PWA-upload path)
    if (app_asset_storage_filename (asset_path, 1, local_filename, sizeof (local_filename)) &&
        http_fs_file_exists_and_nonempty (local_filename))
    {
        if (filename && maxlen) { strncpy (filename, local_filename, maxlen - 1); filename[maxlen - 1] = '\0'; }
        if (gzip_encoded) { *gzip_encoded = 1; }
        rtc = 1;
    }
    else
    {
        // basename .gz  (Arduino LittleFS upload tool — files stored flat in root)
        const char * slash = strrchr (asset_path, '/');
        const char * base  = slash ? slash + 1 : asset_path;

        snprintf (local_filename, sizeof (local_filename), "%s.gz", base);

        if (http_fs_file_exists_and_nonempty (local_filename))
        {
            if (filename && maxlen) { strncpy (filename, local_filename, maxlen - 1); filename[maxlen - 1] = '\0'; }
            if (gzip_encoded) { *gzip_encoded = 1; }
            rtc = 1;
        }
    }

    /* Ein Ausstieg, ein Unmount: Vorher kehrten alle drei Wege ohne Unmount
     * zurueck (L129 b), geraeteweit 28 x begin gegen 27 x end.
     */
    LittleFS.end ();

    return rtc;
}

static int8_t
http_decode_temp_correction (unsigned int value)
{
    return (int8_t) (value & 0xFF);
}

static unsigned int
http_encode_temp_correction (int temp_corr)
{
    return (uint8_t) ((int8_t) temp_corr);
}

static int
http_clamp_temp_correction (int temp_corr)
{
    if (temp_corr < -20)
    {
        temp_corr = -20;
    }
    else if (temp_corr > 20)
    {
        temp_corr = 20;
    }

    return temp_corr;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * A5 (S.4b, design 7.2/7.3): RTC-Temperatur mit Vorzeichen
 *
 * Index 49 (RTC_TEMP_HALF_DEG_NUM_VAR) traegt sie als int16 im Zweierkomplement in halben Grad,
 * 0x8000 = kein Messwert. Index 21 bleibt, wie er ist: 0..250, unter null 0 -- er speist die alte
 * PWA und die Anzeige an der Uhr (Ent-8).
 *
 * Solange der STM Index 49 nicht sendet (alter STM, S.10 bis S.24), steht dort die Vorbelegung
 * 0x8000 aus vars_init(), und die Legacy-Seite zeigt Index 21 wie bisher.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define RTC_TEMP_HALF_DEG_UNKNOWN   0x8000

/* Anzuzeigende RTC-Temperatur in halben Grad: Index 49 mit Vorzeichen, sonst Index 21. */
static int
http_rtc_temp_half_deg (void)
{
    unsigned int    raw = get_numvar (RTC_TEMP_HALF_DEG_NUM_VAR) & 0xFFFF;

    if (raw == RTC_TEMP_HALF_DEG_UNKNOWN)
    {
        return (int) get_numvar (RTC_TEMP_INDEX_NUM_VAR);
    }

    return (int) (int16_t) raw;
}

/* Halbe Grad als Text, vorzeichenrichtig: -3 => "-1.5", -1 => "-0.5", 49 => "24.5", 48 => "24".
 * Gerechnet wird mit dem BETRAG -- "/ 2" und "% 2" auf einem negativen Wert ergaeben -1 und -1,
 * also "-1-.5" bzw. mit floor "-2.5" (derselbe Fehler steckt in formatHalfDegreeValue() der PWA, P3.1).
 */
static void
http_format_half_deg (char * buf, size_t len, int half_deg)
{
    unsigned int    mag = (half_deg < 0) ? (unsigned int) (-half_deg) : (unsigned int) half_deg;

    snprintf (buf, len, "%s%u%s", (half_deg < 0) ? "-" : "", mag / 2, (mag % 2) ? ".5" : "");
}

/* Index 49 nach einer Korrekturaenderung nachrechnen -- im selben Schritt und mit derselben
 * Differenz wie Index 21, an BEIDEN Stellen, die das tun (Legacy "savetcorrrtc" und
 * http_api_temperature_correction_set()). Sonst zeigten Legacy und PWA nach einer Korrektur bis zur
 * naechsten Messung verschiedene Werte (design 7.3: "zwei Wahrheiten").
 *
 * Nur lokal, KEIN set_numvar(): Der Wert gehoert dem STM, er sendet ihn mit der naechsten Messung
 * neu. "Kein Messwert" bleibt "kein Messwert", und 0x8000 wird nie als Ergebnis erzeugt.
 */
static void
http_rtc_temp_half_deg_correct (int delta)
{
    unsigned int    raw = numvars[RTC_TEMP_HALF_DEG_NUM_VAR] & 0xFFFF;
    long            val;

    if (raw == RTC_TEMP_HALF_DEG_UNKNOWN)
    {
        return;
    }

    val = (long) (int16_t) raw - delta;

    if (val < -32767)
    {
        val = -32767;
    }
    else if (val > 32767)
    {
        val = 32767;
    }

    numvars[RTC_TEMP_HALF_DEG_NUM_VAR] = (unsigned int) (uint16_t) (int16_t) val;
}

/* Overlay-Anzahl zum LESEN, begrenzt auf das Feld (S.5, Ergaenzung zu N1).
 *
 * n_overlays kommt ueber die Bruecke und ist auf dieser Seite nicht begrenzt; meldet der STM mehr als
 * MAX_OVERLAYS -- ueber eine verstuemmelte N-Zeile, oder heute ueber Legacy selbst --, laese die Liste
 * hinter overlays[]. Lesestellen begrenzen deshalb, Schreibstellen weisen ab
 * (http_overlays(), http_api_overlay_delete()).
 */
static uint_fast8_t
http_n_overlays_for_read (void)
{
    unsigned int    n = get_numvar (OVERLAY_N_OVERLAYS_NUM_VAR);

    return (n > MAX_OVERLAYS) ? MAX_OVERLAYS : n;
}

static uint_fast8_t
http_app (const char * path)
{
    char            filename[128];
    char            remote_app_version[16];
    const char *    content_type;
    const char *    action;
    uint_fast8_t    is_pwa_index = 0;
    uint_fast8_t    gzip_encoded = 0;
    uint_fast8_t    sent = 0;
    bool            app_complete = false;
    bool            remote_app_available = false;

    if (! strcmp (path, PWA_PREFIX) || ! strcmp (path, PWA_PREFIX "/"))
    {
        if (! http_find_stored_app_asset_filename ("app/index.html", filename, sizeof (filename), &gzip_encoded))
        {
            strncpy (filename, PWA_INDEX_FILE, sizeof (filename) - 1);
            filename[sizeof (filename) - 1] = '\0';
        }

        is_pwa_index = 1;
    }
    else if (! strncmp (path, PWA_PREFIX "/", strlen (PWA_PREFIX "/")))
    {
        char asset_path[128];

        snprintf (asset_path, sizeof (asset_path), "app/%s", path + strlen (PWA_PREFIX "/"));

        if (! http_find_stored_app_asset_filename (asset_path, filename, sizeof (filename), &gzip_encoded))
        {
            http_send (FS("HTTP/1.0 404 Not Found\r\nContent-Type: text/plain\r\n\r\nPWA asset not found\r\n"));
            http_flush ();
            return 0;
        }
    }
    else
    {
        return 0;
    }

    {
        char ct_buf[128];
        size_t ct_len;

        strncpy (ct_buf, filename, sizeof (ct_buf) - 1);
        ct_buf[sizeof (ct_buf) - 1] = '\0';
        ct_len = strlen (ct_buf);

        if (gzip_encoded && ct_len > 3)
        {
            ct_buf[ct_len - 3] = '\0';
        }

        content_type = http_content_type (ct_buf);
    }
    /* Nur unter is_pwa_index: app_complete wird ausschliesslich weiter unten in
     * "is_pwa_index && ! app_complete" und "! is_pwa_index || app_complete" gelesen -
     * bei is_pwa_index == 0 entscheidet in beiden Faellen bereits der erste Operand,
     * der Startwert false aendert dort also nichts. Fuer /app/app.js entfallen damit
     * ein LittleFS-Mount, eine Dateipruefung je Weisslisteneintrag und ein Unmount,
     * deren Ergebnis weggeworfen wurde - unmittelbar vor der grossen
     * Uebertragung (L129 d). Am
     * Geraet gemessen vergehen 239 ms zwischen dem ACK der Anfrage und dem ersten
     * Datenpaket, 18 % der Gesamtzeit (L131).
     */
    if (is_pwa_index)
    {
        app_complete = http_app_installation_complete ();
    }

    action = http_get_param ("action");

    if (is_pwa_index && ! strcmp (action, "install"))
    {
        remote_app_available = http_remote_app_files_available (remote_app_version, sizeof (remote_app_version));

        if (remote_app_available)
        {
            if (http_try_auto_install_app_files ())
            {
                http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nCache-Control: no-cache\r\n\r\n"));
                http_send (FS(
                    "<!DOCTYPE html><html><head><meta charset='utf-8'>"
                    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                    "<meta http-equiv='refresh' content='3;url=/app/'>"
                    "<title>WordClock App wird installiert</title>"
                    "<style>"
                    "body{font-family:Arial,sans-serif;background:#0b1422;color:#eef4ff;padding:24px;line-height:1.5;}"
                    ".card{max-width:680px;margin:0 auto;background:#132033;border:1px solid rgba(255,255,255,.08);"
                    "border-radius:18px;padding:24px;box-shadow:0 20px 50px rgba(0,0,0,.25);}"
                    "h1{margin:0 0 12px;font-size:28px;}p{color:#c5d3ea;}a{display:inline-block;margin:8px 12px 0 0;"
                    "padding:12px 16px;border-radius:999px;text-decoration:none;background:#2d5275;color:#fff;"
                    "border:1px solid rgba(255,255,255,.15);}strong{color:#fff;}"
                    "</style></head><body><div class='card'>"
                    "<h1>WordClock App wird installiert</h1>"
                    "<p>Die WordClock PWA-Dateien wurden einzeln vom Update-Server geladen und im LittleFS gespeichert.</p>"
                    "<p><strong>Weiter so:</strong> Die Seite wechselt gleich automatisch nach <code>/app</code>.</p>"
                    "<a href='/app/'>Zur App</a>"
                    "<a href='/legacy'>Zur Legacy-Seite</a>"
                    "</div></body></html>"
                ));
                http_flush ();
                return 0;
            }

            http_send (FS("HTTP/1.0 500 Internal Server Error\r\nContent-Type: text/plain; charset=utf-8\r\nCache-Control: no-cache\r\n\r\n"));
            http_send (FS("WordClock app file installation failed.\r\n"));
            http_flush ();
            return 0;
        }

        http_send (FS("HTTP/1.0 404 Not Found\r\nContent-Type: text/plain; charset=utf-8\r\nCache-Control: no-cache\r\n\r\n"));
        http_send (FS("WordClock app files are not available on the configured update server.\r\n"));
        http_flush ();
        return 0;
    }

    if (is_pwa_index && ! app_complete)
    {
        remote_app_available = http_remote_app_files_available (remote_app_version, sizeof (remote_app_version));
    }

    if (! is_pwa_index || app_complete)
    {
        sent = http_send_fs_file (filename, content_type, gzip_encoded);
    }

    if (sent)
    {
        return 0;
    }

    if (! sent)
    {
        if (is_pwa_index)
        {
            http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nCache-Control: no-cache\r\n\r\n"));
            http_send (FS(
                "<!DOCTYPE html><html><head><meta charset='utf-8'>"
                "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                "<title>WordClock App noch nicht installiert</title>"
                "<style>"
                "body{font-family:Arial,sans-serif;background:#0b1422;color:#eef4ff;padding:24px;line-height:1.5;}"
                ".card{max-width:680px;margin:0 auto;background:#132033;border:1px solid rgba(255,255,255,.08);"
                "border-radius:18px;padding:24px;box-shadow:0 20px 50px rgba(0,0,0,.25);}"
                "h1{margin:0 0 12px;font-size:28px;}p{color:#c5d3ea;}a{display:inline-block;margin:8px 12px 0 0;"
                "padding:12px 16px;border-radius:999px;text-decoration:none;background:#2d5275;color:#fff;"
                "border:1px solid rgba(255,255,255,.15);}strong{color:#fff;}code{color:#fff;}"
                "</style></head><body><div class='card'>"
            ));

            if (remote_app_available)
            {
                http_send (FS("<h1>WordClock App ist lokal nicht installiert</h1>"));
                http_send (FS("<p>Auf diesem Gerät fehlen noch die App-Dateien im LittleFS, aber auf dem konfigurierten Update-Server ist eine WordClock PWA-Version verfügbar.</p>"));
                http_send (FS("<p><strong>Server-App verfügbar:</strong> "));
                if (remote_app_version[0])
                {
                    http_send (remote_app_version);
                }
                else
                {
                    http_send (FS("ja"));
                }
                http_send (FS("</p>"));
                http_send (FS("<p><strong>Weiter so:</strong> Die Erstinstallation wird direkt hier ueber die Zwischenmaske gestartet.</p>"));
                http_send (FS("<a href='/app/?action=install'>App jetzt installieren</a>"));
                http_send (FS("<a href='/fs'>Zu Dateien</a>"));
                http_send (FS("<a href='/update'>Zu Update</a>"));
                http_send (FS("<a href='/legacy'>Zur Legacy-Seite</a>"));
            }
            else
            {
                http_send (FS("<h1>WordClock App ist noch nicht installiert</h1>"));
                http_send (FS("<p>Auf diesem Gerät wurden noch keine vollständigen App-Dateien in das LittleFS geladen.</p>"));
                http_send (FS("<p><strong>Weiter so:</strong> Update-Host/-Pfad pruefen und die Erstinstallation danach wieder hier ueber <code>/app</code> anstossen.</p>"));
                http_send (FS("<a href='/fs'>Zu Dateien</a>"));
                http_send (FS("<a href='/update'>Zu Update</a>"));
                http_send (FS("<a href='/legacy'>Zur Legacy-Seite</a>"));
            }

            http_send (FS("</div></body></html>"));
        }
        else
        {
            http_send (FS("HTTP/1.0 404 Not Found\r\nContent-Type: text/plain\r\n\r\nPWA asset not found\r\n"));
        }
        http_flush ();
    }

    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * normalize http parameters
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
normalize_http_parameters (char * p)
{
    if (! p)                                                // ein Parameter ohne '=' hat keinen Wert
    {
        return;
    }

    while (*p)
    {
        if (*p == '%')
        {
            char * pp;

            *p = htoi (p + 1, 2);

            for (pp = p + 1; *(pp + 2); pp++)
            {
                *pp = *(pp + 2);
            }
            *pp = '\0';
        }
        p++;
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set parameters from list
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
http_set_params (char * paramlist)
{
    char *  p;                                              // ap=access&pw=secret&action=saveap
    int     idx = 0;
    int     i;

    if (paramlist && *paramlist)
    {
        http_parameters[idx].name = paramlist;
        http_parameters[idx].value = (char *) 0;            // siehe unten

        for (p = paramlist; idx < MAX_HTTP_PARAMS - 1 && *p; p++)
        {
            if (*p == '=')
            {
                *p = '\0';
                http_parameters[idx].value = p + 1;
            }
            else if (*p == '&')
            {
                *p = '\0';
                idx++;
                http_parameters[idx].name = p + 1;

                /* Ohne dieses Zuruecksetzen behaelt .value den Wert des vorherigen
                 * Parameters -- und beim ersten Request nach dem Start zeigt es ins
                 * Nichts. Eine Anfrage wie "GET /?a" (Parameter ohne '=') liess
                 * normalize_http_parameters() dann in einen Stackpuffer schreiben,
                 * dessen Frame laengst weg war. Aus dem ganzen LAN ausloesbar,
                 * ohne Anmeldung.
                 */
                http_parameters[idx].value = (char *) 0;
            }
        }
        idx++;
    }

    for (i = 0; i < idx; i++)
    {
        normalize_http_parameters (http_parameters[i].value);
    }

    while (idx < MAX_HTTP_PARAMS)
    {
        http_parameters[idx].name = (char *) 0;
        idx++;
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * get a parameter
 *
 * VERTRAG (C20/L199): Diese Funktion liefert NIE NULL. Fehlt der Parameter, kommt ein
 * leerer String. Aufrufer pruefen deshalb auf *value, nicht auf value.
 *
 * Bis zum 05.10.2026 stimmte dieser Vertrag NICHT, und der Befund hatte es umgekehrt:
 * L199 und L273 fuehrten "gibt nie NULL zurueck" als die URSACHE und die NULL-Pruefungen
 * als tote Zweige. Nachgelesen hat sich das Gegenteil gezeigt -- http_set_params() setzt
 * .value auf NULL, wenn ein Parameter OHNE '=' ankommt, und gab genau dieses NULL hier
 * heraus. Damit war
 *
 *     GET /update?action=flash&stm32_filenames      -> strncpy (dst, NULL, 63)
 *     GET /api/dfplayer_alarm_set?idx               -> atoi (NULL)
 *
 * aus dem ganzen LAN ohne Anmeldung ausloesbar. Die rund 144 Aufrufstellen pruefen
 * ueberwiegend gar nicht, und ein Teil schiebt den Rueckgabewert roh in atoi(), strcmp()
 * oder strncpy(). Es ist deshalb der Vertrag, der eingeloest wird, und nicht jede
 * Aufrufstelle, die eine Pruefung bekommt -- eine einzige vergessene reichte.
 *
 * Wer das hier je zurueckdreht, dreht beide Abstuerze wieder auf.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static char *
http_get_param (const char * name)
{
    static char empty[] = "";
    int     idx;

    for (idx = 0; idx < MAX_HTTP_PARAMS && http_parameters[idx].name != (char *) 0; idx++)
    {
        if (! strcmp (http_parameters[idx].name, name))
        {
            return http_parameters[idx].value ? http_parameters[idx].value : empty;
        }
    }

    return empty;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * get a parameter by index
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static char *
http_get_param_by_idx (const char * name, int idx)
{
    char    name_buf[16];
    char *  rtc;

    sprintf (name_buf, "%s%d", name, idx);
    rtc = http_get_param (name_buf);

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * get a checkbox parameter
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static int
http_get_checkbox_param (const char * name)
{
    char *  value = http_get_param (name);
    int     rtc;

    if (! strcmp (value, "active"))
    {
        rtc = 1;
    }
    else
    {
        rtc = 0;
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * get a checkbox parameter by index
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static int
http_get_checkbox_param_by_idx (const char * name, int idx)
{
    char *  value = http_get_param_by_idx (name, idx);
    int     rtc;

    if (! strcmp (value, "active"))
    {
        rtc = 1;
    }
    else
    {
        rtc = 0;
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * send http style
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
http_style (void)
{
    http_send_FS ("<style>\r\n");
    http_send_FS ("* { padding: 0; margin: 0; FONT-FAMILY: Verdana,Helvetica,sans-serif; FONT-SIZE: 14px}\r\n");
    http_send_FS ("BODY, HTML {height: 100%}\r\n");
    http_send_FS ("H1 {FONT-SIZE: 24px}\r\n");
    http_send_FS ("H2 {FONT-SIZE: 18px}\r\n");
    http_send_FS ("H3 {FONT-SIZE: 16px}\r\n");
    http_send_FS ("TH {color: darkblue}\r\n");
    http_send_FS ("A {text-decoration: none}\r\n");
    http_send_FS (".nav-pane {color: #ffffff}\r\n");
    http_send_FS (".content-pane {color: #000000}\r\n");
    http_send_FS (".nav-pane a:link, .nav-pane a:visited {background: none; color: #eeee00}\r\n");
    http_send_FS (".nav-pane a:hover {background: none; color: #ffffff}\r\n");
    http_send_FS (".nav-pane a:active {background: none; color: #ffff00}\r\n");
    http_send_FS (".content-pane a:link {background: none; color: #0000cc}\r\n");
    http_send_FS (".content-pane a:visited {background: none; color: #551a8b}\r\n");
    http_send_FS (".content-pane a:hover {background: none; color: #000099}\r\n");
    http_send_FS (".content-pane a:active {background: none; color: #cc0000}\r\n");
    http_send_FS ("SELECT,BUTTON,.button,.custom-file-upload\r\n");
    http_send_FS ("{\r\n");
    http_send_FS (" background: none;\r\n");
    http_send_FS (" border: 1px solid;\r\n");
    http_send_FS (" border-color: #0000ff;\r\n");
    http_send_FS (" color: #0000ff;\r\n");
    http_send_FS (" padding: 4px 8px;\r\n");
    http_send_FS (" text-align: center;\r\n");
    http_send_FS (" text-decoration: none;\r\n");
    http_send_FS (" display: inline-block;\r\n");
    http_send_FS (" font-size: 12px;\r\n");
    http_send_FS (" margin: 4px 2px;\r\n");
    http_send_FS (" cursor: pointer;\r\n");
    http_send_FS ("}\r\n");
    http_send_FS ("BUTTON:hover\r\n");
    http_send_FS ("{\r\n");
    http_send_FS (" background-color: #0000B0;\r\n");
    http_send_FS (" color: #ffff00;\r\n");
    http_send_FS ("}\r\n");
    http_send_FS ("input[type=\"file\"] { display: none; }");
    http_send_FS (".bigtable {height: 100%}\r\n");
    http_send_FS ("</style>\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * send http and html header
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
http_header (const char * title, const char * refresh, const char * url)
{
    http_send_FS ("HTTP/1.0 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nCache-Control: no-cache\r\n\r\n<!DOCTYPE html>\r\n<html><head><meta charset=\"utf-8\"><title>");

    http_send (pgm_name);

    if (title && *title)
    {
        http_send (" ");
        http_send (title);
    }

    http_send_FS ("</title>");

    if (refresh)
    {
        http_send_FS ("<meta http-equiv=\"refresh\" content=\"");
        http_send (refresh);
        http_send_FS ("; URL=");
        http_send (url);
        http_send_FS ("\">");
    }

    http_style ();
    http_send_FS ("</head><body>\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * send html trailer
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
http_trailer (void)
{
    http_send_FS ("</body></html>\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * send table header columns
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_header (const char ** columns, int cols)
{
    int i;

    http_send_FS ("<table border=0>");

    if (columns && cols)
    {
        http_send_FS ("<tr>");

        for (i = 0; i < cols; i++)
        {
            http_send_FS ("<th style='textalign:left'><font color=darkblue>");
            http_send (columns[i]);
            http_send_FS ("</font></th>");
        }

        http_send_FS ("</tr>\r\n");
    }
    bgcolor_cnt = 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * begin form
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
begin_form (const char * page)
{
    http_send_FS ("<form method=\"GET\" action=\"/");
    http_send (page);
    http_send_FS ("\">\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * end form
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
end_form (void)
{
    http_send_FS ("</form>\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * begin table row
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
begin_table_row (int show_bgcolor = 1)
{
    bgcolor_cnt++;

    if (show_bgcolor && bgcolor_cnt & 0x01)
    {
        http_send_FS ("<tr bgcolor=#f0f0f0>\r\n");
    }
    else
    {
        http_send_FS ("<tr>\r\n");
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * begin table row as form
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
begin_table_row_form (const char * page, int show_bgcolor = 1)
{
    begin_table_row (show_bgcolor);
    begin_form (page);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * end table row
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
end_table_row (void)
{
    http_send_FS ("</tr>\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * end table row as form
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
end_table_row_form (void)
{
    end_form ();
    end_table_row ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * checkbox field
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
checkbox_field (const char * id, const char * desc, int checked)
{
    http_send (desc);
    http_send_FS ("&nbsp;<input type=\"checkbox\" name=\"");
    http_send (id);
    http_send_FS ("\" value=\"active\" ");

    if (checked)
    {
        http_send_FS ("checked");
    }

    http_send_FS (">");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * begin column
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
begin_column (void)
{
    http_send_FS ("<td>");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * end column
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
end_column (void)
{
    http_send_FS ("</td>");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * checkbox column
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
checkbox_column (const char * id, const char * desc, int checked)
{
    begin_column ();
    checkbox_field (id, desc, checked);
    end_column ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * text column
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
text_column (const char * text)
{
    begin_column ();
    http_send (text);
    end_column ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * text column (align right)
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
text_rcolumn (const char * text)
{
    http_send_FS ("<td align=\"right\">");
    http_send (text);
    end_column ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * input field
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
input_field (const char * id, const char * desc, const char * value, int maxlength, int maxsize)
{
    char maxlength_buf[8];
    char maxsize_buf[8];

    sprintf (maxlength_buf, "%d", maxlength);
    sprintf (maxsize_buf, "%d", maxsize);

    if (desc && *desc)
    {
        http_send (desc);
        http_send_FS ("&nbsp;");
    }

    http_send_FS ("<input type=\"text\" id=\"");
    http_send (id);
    http_send_FS ("\" name=\"");
    http_send (id);
    http_send_FS ("\" value=\"");
    http_send (value);
    http_send_FS ("\" maxlength=\"");
    http_send (maxlength_buf);
    http_send_FS ("\" size=\"");
    http_send (maxsize_buf);
    http_send_FS ("\">");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * input column
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
input_column (const char * id, const char * desc, const char * value, int maxlength, int maxsize)
{
    begin_column ();
    input_field (id, desc, value, maxlength, maxsize);
    end_column ();
}

static void
input_column (const char * id, const char * desc, int value, int maxlength, int maxsize)
{
    char buf[32];

    sprintf (buf, "%d", value);
    input_column (id, desc, buf, maxlength, maxsize);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * select field
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
select_field (const char * id, const char ** text, int selected_value, int min_value, int max_value, int autosubmit)
{
    char    buf[16];
    int     i;

    http_send_FS ("<select id=\"");
    http_send (id);
    http_send_FS ("\" name=\"");
    http_send (id);

    if (autosubmit)
    {
         http_send_FS ("\" onchange='this.form.submit()'>");
    }
    else
    {
        http_send_FS ("\">");
    }

    for (i = min_value; i < max_value; i++)
    {
        sprintf (buf, "%d", i);
        http_send_FS ("<option value=\"");
        http_send (buf);
        http_send_FS ("\"");

        if (i == selected_value)
        {
            http_send_FS (" selected");
        }

        http_send_FS (">");
        http_send ((char *) text[i]);
        http_send_FS ("</option>");
    }

    http_send_FS ("</select>\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * select column
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
select_column (const char * id, const char ** text, int selected_value, int min_value, int max_value, int autosubmit)
{
    begin_column ();
    select_field (id, text, selected_value, min_value, max_value, autosubmit);
    end_column ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * slider field
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
slider_field (const char * id, const char * text, const char * value, const char * min, const char * max, const char * pixelwidth)
{
    if (text && *text)
    {
        http_send (text);
        http_send_FS ("&nbsp;");
    }

    http_send_FS ("<input type=\"range\" id=\"");
    http_send (id);
    http_send_FS ("\" name=\"");
    http_send (id);
    http_send_FS ("\" value=\"");
    http_send (value);
    http_send_FS ("\" min=\"");
    http_send (min);
    http_send_FS ("\" max=\"");
    http_send (max);

    if (pixelwidth && *pixelwidth)
    {
        http_send_FS ("\" style=\"width:");
        http_send (pixelwidth);
        http_send_FS ("px;");
    }

    http_send_FS ("\" oninput=\"");
    http_send (id);
    http_send_FS ("_output.value=");
    http_send (id);
    http_send_FS (".value\">");
    http_send_FS ("<output name=\"");
    http_send (id);
    http_send_FS ("_output\">");
    http_send (value);
    http_send_FS ("</output>");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * slider column
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
slider_column (const char * id, const char * value, const char * min, const char * max)
{
    http_send_FS ("<td>");
    slider_field (id, "", value, min, max, "");
    http_send_FS ("</td>\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * submit button field
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
button_field (const char * id, const char * text, const char * style = NULL)
{
    http_send_FS ("<button type=\"submit\" name=\"action\" ");

    if (style)
    {
        http_send (style);
    }

    http_send_FS (" value=\"");
    http_send (id);
    http_send_FS ("\">");
    http_send (text);
    http_send_FS ("</button>&nbsp;");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * submit column field
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
button_column (const char * id, const char * text, const char * style = NULL)
{
    begin_column ();
    button_field (id, text, style);
    end_column ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save column
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
save_column (const char * id)
{
    http_send_FS ("<td><button type=\"submit\" name=\"action\" value=\"save");
    http_send (id);
    http_send_FS ("\">Save</button></td>\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * table row
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_row (const char * col1, const char * col2, const char * col3)
{
    begin_table_row ();
    text_column (col1);
    text_column (col2);
    text_column (col3);
    end_table_row ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * table row with input
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_row_input (const char * page, int cols, const char * text, const char * id, const char * value, int maxlength, int show_bgcolor = 1)
{
    int cols_used = 1;
    int maxsize = maxlength;

    if (maxsize > 32)
    {
        maxsize = 32;
    }

    begin_table_row_form (page, show_bgcolor);
    text_column (text);
    cols_used++;
    input_column (id, "", value, maxlength, maxsize);
    cols_used++;

    while (cols_used < cols)
    {
        http_send_FS ("<td></td>");
        cols_used++;
    }

    save_column (id);
    end_table_row_form ();
}

static void
table_row_input (const char * page, int cols, const char * text, const char * id, int value, int maxlength)
{
    char buf[32];
    sprintf (buf, "%d", value);
    table_row_input (page, cols, text, id, buf, maxlength);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * table row with input and additional button
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#if 0
static void
table_row_input_with_button (const char * page, int cols, const char * text, const char * id, const char * value, int maxlength, const char * buttonid, const char * buttontext)
{
    int cols_used = 1;

    begin_table_row_form (page);
    text_column (text);
    cols_used++;
    input_column (id, "", value, maxlength, maxlength);
    cols_used++;
    button_column (buttonid, buttontext);
    cols_used++;

    while (cols_used < cols)
    {
        http_send_FS ("<td></td>");
        cols_used++;
    }

    save_column (id);
    end_table_row_form ();
}
#endif
/*-------------------------------------------------------------------------------------------------------------------------------------------
 * table row with multiple inputs
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_row_inputs (const char * page, const char * id, int n, const char ** ids, const char ** desc, const char ** value, int * maxlength, int * maxsize)
{
    int     i;

    begin_table_row_form (page);

    for (i = 0; i < n; i++)
    {
        http_send_FS ("<td>");
        input_field (ids[i], desc[i], value[i], maxlength[i], maxsize[i]);
        http_send_FS ("</td>");
    }

    save_column (id);
    end_table_row_form ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * table row with checkbox
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_row_checkbox (const char * page, const char * text, const char * id, const char * desc, int checked)
{
    begin_table_row_form (page);
    text_column (text);
    checkbox_column (id, desc, checked);
    save_column (id);
    end_table_row_form ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * table row with selection
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_row_select (const char * page, const char * text1, const char * id, const char ** text2, int selected_value, int min_value, int max_value, int autosubmit)
{
    begin_table_row_form (page);
    text_column (text1);
    select_column (id, text2, selected_value, min_value, max_value, autosubmit);
    save_column (id);
    end_table_row_form ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * table row with slider
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_row_slider (const char * page, const char * text1, const char * id, const char * text2, const char * min, const char * max)
{
    begin_table_row_form (page);
    text_column (text1);
    slider_column (id, text2, min, max);
    save_column (id);
    end_table_row_form ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * table row with n sliders
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_row_sliders (const char * page, const char * text1, const char * id, int n, const char ** ids, const char ** desc, const char * const * text2, const char ** min, const char ** max)
{
    int i;

    begin_table_row_form (page);
    text_column (text1);

    http_send_FS ("<td>");

    for (i = 0; i < n; i++)
    {
        slider_field (ids[i], desc[i], text2[i], min[i], max[i], "64");
        http_send_FS ("&nbsp;&nbsp;&nbsp;\r\n");
    }

    http_send_FS ("</td>\r\n");

    save_column (id);
    end_table_row_form ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * table trailer
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_trailer (void)
{
    http_send_FS ("</table><P>\r\n");
}

static void
menu_entry (const char * page, const char * entry)
{
    http_send_FS ("<a href=\"/");
    http_send (page);
    http_send_FS ("\" style=\"text-decoration: none\">");
    http_send (entry);
    http_send_FS ("</a><BR>");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * menu
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
begin_box (const char * title)
{
    http_send_FS ("<table class=\"bigtable\"><tr><td class=\"nav-pane\" style=\"padding:10px\" valign=\"top\" bgcolor=\"#000080\">"
                  "<font color=white><H1>");

    http_send (pgm_name);

    http_send_FS ("</H1></font><BR>\r\n");

    menu_entry ("", "Main");
    menu_entry ("network", "Network");
    menu_entry ("temperature", "Temperature");
    menu_entry ("weather", "Weather");
    menu_entry ("ldr", "LDR");
    menu_entry ("dispbright", "Brightness");
    menu_entry ("ambibright", "Ambilight Brightness");
    menu_entry ("display", "Display");
    menu_entry ("animations", "Animations");
    menu_entry ("overlays", "Overlays");
    menu_entry ("ambilight", "Ambilight");
    menu_entry ("timers", "Timers");
    menu_entry ("atimers", "Ambilight Timers");
    menu_entry ("dfplayer", "DFPlayer");

    if (hardware_configuration != 0xFFFF && (hardware_configuration & HW_LED_MASK) == HW_LED_TFTLED_RGB_LED)
    {
        menu_entry ("tft", "TFT");
    }

    menu_entry ("fs", "Files");
    menu_entry ("update", "Update");
    menu_entry ("flash_stm32_local", "Local Update");
    http_send_FS ("<a href=\"/app/\" style=\"text-decoration: none\"><font color=#ffff00><B>New App</B></font></a><BR>");

    http_send_FS ("</td><td class=\"content-pane\" style=\"padding:10px\"  align=\"left\" valign=\"top\">\r\n");      // fm: center?

    if (title && *title)
    {
        http_send_FS ("<H1>");
        http_send (title);
        http_send_FS ("</H1><BR>");
    }
}

static void
end_box (void)
{
    http_send_FS ("</td></tr></table>\r\n");
}

static uint_fast8_t
icon_files_exist (void)
{
    static const char * const icon_candidates[] = { "wc24h-icon.txt", "wc12h-icon.txt", "uc-icon.txt" };
    static const char * const weather_candidates[] = { "wc24h-weather.txt", "wc12h-weather.txt", "uc-weather.txt" };
    const char *    fname_icon;
    const char *    fname_weather;
    uint_fast8_t    rtc = 0;

    LittleFS.begin();
    fname_icon = http_find_existing_filename (http_get_configured_icon_filename (), icon_candidates, sizeof (icon_candidates) / sizeof (icon_candidates[0]));
    fname_weather = http_find_existing_filename (http_get_configured_weather_filename (), weather_candidates, sizeof (weather_candidates) / sizeof (weather_candidates[0]));

    if (fname_icon && fname_weather && LittleFS.exists(fname_icon) && LittleFS.exists(fname_weather))
    {
        rtc = 1;
    }

    LittleFS.end();
    return rtc;
}

static void
message_icon_files_missing (void)
{
    if (! icon_files_exist ())
    {
        http_send_FS ("<font color=\"red\"><B>Please install the icon files onto <a href=\"fs\">FS</a></B></font><P>\r\n");
    }
}

static void
message_tables_file_missing (void)
{
    if (! tables_fname())
    {
        if (hardware_configuration != 0xFFFF && (hardware_configuration & HW_WC_MASK) != HW_UCLOCK)
        {
            http_send_FS ("<font color=\"red\"><B>Please install the layout table file wcxx-tables-xx.txt onto <a href=\"fs\">FS</a></B></font><P>\r\n");
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * main page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_main (void)
{
    const char *    thispage = "";
    const char *    header_cols[MAIN_HEADER_COLS]               = { "Name", "Value" };
    const char *    datetime_header_cols[DATETIME_HEADER_COLS]  = { "YYYY", "MM", "DD", "hh", "mm", "Action" };
    const char *    ids[5]                                      = { "year", "month", "day", "hour", "min" };
    const char *    desc[5]                                     = { "", "", "", "", "" };
    int             maxlen[5]                                   = { 4, 2, 2, 2, 2 };
    int             maxsize[5]                                  = { 4, 2, 2, 2, 2 };
    char            year_str[16];
    char            mon_str[16];
    char            day_str[16];
    char            hour_str[16];
    char            minutes_str[16];
    const char *    values[5]                                   = { year_str, mon_str, day_str, hour_str, minutes_str };
    char *          action;
    const char *    message                                     = (const char *) 0;
    const char *    alert_message                               = (const char *) 0;
    STR_VAR *       sv;
    const char *    esp_firmware_version                        = ESP_VERSION;
    char *          version;
    char *          eeprom_version;
    uint_fast8_t    rtc                                         = 0;
    struct tm *     tmp;
    uint_fast8_t    eeprom_is_up;
    uint_fast8_t    rtc_is_up;
    uint_fast8_t    dfplayer_is_up;
    uint_fast16_t   dfplayer_version;
    uint_fast8_t    do_reset = 0;
    uint_fast8_t    do_reset_eeprom = 0;

    sv              = get_strvar (VERSION_STR_VAR);
    version         = sv->str;

    sv              = get_strvar (EEPROM_VERSION_STR_VAR);
    eeprom_version  = sv->str;

    tmp = get_tm_var (CURRENT_TM_VAR);

    if (tmp->tm_year >= 0 && tmp->tm_mon >= 0 && tmp->tm_mday >= 0 && tmp->tm_hour >= 0 && tmp->tm_min >= 0 &&
        tmp->tm_year <= 1200 && tmp->tm_mon <= 12 && tmp->tm_mday <= 31 && tmp->tm_hour < 24 && tmp->tm_min < 60)
    {                                                               // check values to avoid buffer overflow
        sprintf (year_str,      "%4d",  tmp->tm_year + 1900);
        sprintf (mon_str,       "%02d", tmp->tm_mon + 1);
        sprintf (day_str,       "%02d", tmp->tm_mday);
        sprintf (hour_str,      "%02d", tmp->tm_hour);
        sprintf (minutes_str,   "%02d", tmp->tm_min);
    }
    else
    {
        year_str[0]     = '\0';
        mon_str[0]      = '\0';
        day_str[0]      = '\0';
        hour_str[0]     = '\0';
        minutes_str[0]  = '\0';
    }

    rtc_is_up         = get_numvar (RTC_IS_UP_NUM_VAR);
    eeprom_is_up      = get_numvar (EEPROM_IS_UP_NUM_VAR);
    dfplayer_is_up    = get_numvar (DFPLAYER_IS_UP_NUM_VAR);
    dfplayer_version  = get_numvar (DFPLAYER_VERSION_NUM_VAR);

    action = http_get_param ("action");

    if (*action)
    {
        if (! strcmp (action, "learnir"))
        {
            message = "Learning IR remote control...";
            rpc (LEARN_IR_RPC_VAR);
        }
        else if (! strcmp (action, "poweron"))
        {
            message = "Switching power on...";
            set_numvar (DISPLAY_POWER_NUM_VAR, 1);
        }
        else if (! strcmp (action, "poweroff"))
        {
            message = "Switching power off...";
            set_numvar (DISPLAY_POWER_NUM_VAR, 0);
        }
        else if (! strcmp (action, "apoweron"))
        {
            message = "Switching ambilight power on...";
            set_numvar (DISPLAY_AMBILIGHT_POWER_NUM_VAR, 1);
        }
        else if (! strcmp (action, "apoweroff"))
        {
            message = "Switching ambilight power off...";
            set_numvar (DISPLAY_AMBILIGHT_POWER_NUM_VAR, 0);
        }
        else if (! strcmp (action, "rststm32"))
        {
            message = "Resetting STM32...";
            do_reset = 1;
        }
        else if (! strcmp (action, "rsteeprom"))
        {
            do_reset_eeprom = 1;
        }
        else if (! strcmp (action, "rsteeprom2"))
        {
            message = "Resetting EEPROM, you should now restart STM32...";
            rpc (RESET_EEPROM_RPC_VAR);
            do_reset_eeprom = 2;
        }
        else if (! strcmp (action, "savedatetime"))
        {
            TM tm;

            int year    = 0;
            int month   = 0;
            int day     = 0;
            int hour    = 0;
            int minutes = 0;

            /* Leere Felder duerfen nicht als 0 durchgehen - sonst stellt ein nur halb
             * ausgefuelltes Formular die Uhr auf das Jahr 0 (L29). Bewusst nicht mit &&
             * verkettet: Jedes Feld wird gelesen, damit nach einer Abweisung alle
             * uebrigen Eingaben im Formular stehen bleiben.
             */
            uint_fast8_t fields_ok = 1;

            fields_ok &= http_get_int_param ("year", &year);
            fields_ok &= http_get_int_param ("month", &month);
            fields_ok &= http_get_int_param ("day", &day);
            fields_ok &= http_get_int_param ("hour", &hour);
            fields_ok &= http_get_int_param ("min", &minutes);

            /* Die Eingabe bleibt im Formular stehen, auch wenn sie abgewiesen wird -
             * sonst muesste der Nutzer alle fuenf Felder neu tippen.
             */
            sprintf (year_str,      "%4d",  year);
            sprintf (mon_str,       "%02d", month);
            sprintf (day_str,       "%02d", day);
            sprintf (hour_str,      "%02d", hour);
            sprintf (minutes_str,   "%02d", minutes);

            /* Dieselbe Kalenderpruefung wie in http_api_datetime_set: Jedes Feld nur
             * einzeln zu begrenzen laesst den 31. Februar durch (L32).
             */
            if (! fields_ok ||
                year < 2000 || year > 2999 ||
                month < 1 || month > 12 ||
                day < 1 || day > (int) http_days_in_month (year, month) ||
                hour < 0 || hour > 23 ||
                minutes < 0 || minutes > 59)
            {
                alert_message = "Invalid date or time - clock not changed!";
            }
            else
            {
                tm.tm_year  = year - 1900;
                tm.tm_mon   = month - 1;
                tm.tm_mday  = day;
                tm.tm_hour  = hour;
                tm.tm_min   = minutes;
                tm.tm_sec   = 0;
                tm.tm_wday  = dayofweek (day, month, year);

                set_tm_var (CURRENT_TM_VAR, &tm);
            }
        }
        else if (! strcmp (action, "saveticker"))
        {
            char * ticker = http_get_param ("ticker");
            set_strvar (TICKER_TEXT_STR_VAR, ticker);
        }
    }

    if (do_reset)
    {
        http_header ("Reset", "20", "/");
    }
    else
    {
        http_header ("", (const char *) NULL, (const char *) NULL);
    }

    begin_box ("Main");

    if (do_reset_eeprom == 1)
    {
        begin_form (thispage);
        http_send_FS ("<P><B>Are you really shure to set all EEPROM values to factory settings?</B>\r\n");
        button_field ("rsteeprom2", "YES, Reset EEPROM!");
        end_form ();
        end_box ();
        http_trailer ();
        http_flush ();
    }
    else if (do_reset_eeprom == 2)
    {
        begin_form (thispage);
        http_send_FS ("<P><B>You should now restart STM32</B><BR>\r\n");
        button_field ("rststm32", "Restart STM32");
        end_form ();
        end_box ();
        http_trailer ();
        http_flush ();
    }
    else if (do_reset)
    {
        http_send_FS ("<P><B>Resetting STM32, reconnecting in 20 seconds ...</B><BR>\r\n");
        end_box ();
        http_trailer ();
        http_flush ();
        delay (200);
        stm32_reset ();
    }
    else
    {
        const char * proc     = "unknown";
        const char * osc      = "unknown";
        const char * freq     = "unknown";
        const char * board    = "unknown";
        const char * led_type = "unknown";

        switch (hardware_configuration & HW_STM32_MASK)
        {
            case HW_STM32_F103C8:   proc = "STM32F103C8"; board = "BluePill";   freq = "72MHz";   break;
            case HW_STM32_F401RE:   proc = "STM32F401RE"; board = "Nucleo";     freq = "84MHz";   break;
            case HW_STM32_F411RE:   proc = "STM32F411RE"; board = "Nucleo";     freq = "100MHz";  break;
            case HW_STM32_F446RE:   proc = "STM32F446RE"; board = "Nucleo";     freq = "180MHz";   break;
            case HW_STM32_F407VE:   proc = "STM32F407VE"; board = "BlackBoard"; freq = "168MHz";   break;
            case HW_STM32_F401CC:   proc = "STM32F401CC"; board = "BlackPill";  freq = "84MHz";   break;
            case HW_STM32_F411CE:   proc = "STM32F411CE"; board = "BlackPill";  freq = "100MHz";  break;
        }

        switch (hardware_configuration & HW_OSC_FREQUENCY_MASK)
        {
            case HW_OSC_FREQUENCY_8MHZ:   osc = "8 MHz";            break;
            case HW_OSC_FREQUENCY_25MHZ:  osc = "25 MHz";           break;
        }
        switch (hardware_configuration & HW_LED_MASK)
        {
            case HW_LED_WS2812_GRB_LED:   led_type = "WS2812 GRB";  break;
            case HW_LED_WS2812_RGB_LED:   led_type = "WS2812 RGB";  break;
            case HW_LED_APA102_RGB_LED:   led_type = "APA102 GRB";  break;
            case HW_LED_SK6812_RGB_LED:   led_type = "SK6812 RGB";  break;
            case HW_LED_SK6812_RGBW_LED:  led_type = "SK6812 RGBW"; break;
            case HW_LED_TFTLED_RGB_LED:   led_type = "TFT RGB";     break;
        }

        message_tables_file_missing ();

        http_send_FS ("<table><tr valign=\"top\"><td>\r\n");
        table_header (header_cols, MAIN_HEADER_COLS);
        table_row ("Board", board, "");
        table_row ("Processor", proc, "");
        table_row ("Oscillator", osc, "");
        table_row ("Frequency", freq, "");
        table_row ("Hardware", hardware, "");
        table_row ("Display", led_type, "");
        table_row ("Version", version, "");
        table_row ("ESP8266 version", esp_firmware_version, "");
        table_trailer ();

        http_send_FS ("</td><td>\r\n");
        table_header (header_cols, MAIN_HEADER_COLS);
        table_row ("RTC", rtc_is_up ? "online" : "offline", "");
        table_row ("EEPROM", eeprom_is_up ? "online" : "offline", "");

        if (eeprom_is_up)
        {
            table_row ("EEPROM Version", eeprom_version, "");
        }

        table_row ("DFPlayer", dfplayer_is_up ? "online" : "offline", "");

        if (dfplayer_is_up)
        {
            char dfplayer_version_buf[16];
    
            sprintf (dfplayer_version_buf, "%04x", dfplayer_version);
            table_row ("DFPlayer Version", dfplayer_version_buf, "");
        }

        table_row ("Display power", get_numvar (DISPLAY_POWER_NUM_VAR) ? "on" : "off", "");
        table_trailer ();
        http_send_FS ("</td></tr></table><BR>\r\n");

        table_header (datetime_header_cols, DATETIME_HEADER_COLS);
        table_row_inputs (thispage, "datetime", 5, ids, desc, values, maxlen, maxsize);
        table_trailer ();

        table_header (header_cols, TICKER_HEADER_COLS);
        table_row_input (thispage, 3, "Ticker", "ticker", "", MAX_TICKER_TEXT_LEN);
        table_trailer ();

        begin_form (thispage);
        table_header ((const char **) 0, 0);
        begin_table_row (0);
        button_column ("poweron", "Power On", "style=\"width:180px\"");
        button_column ("poweroff", "Power Off", "style=\"width:180px\"");
        end_table_row ();
        begin_table_row (0);
        button_column ("apoweron", "Ambilight Power On", "style=\"width:180px\"");
        button_column ("apoweroff", "Ambilight Power Off", "style=\"width:180px\"");
        end_table_row ();
        begin_table_row (0);
        button_column ("learnir", "Learn IR remote control", "style=\"width:180px\"");
        button_column ("eepromdump", "EEPROM dump", "style=\"width:180px\"");
        end_table_row ();
        begin_table_row (0);
        button_column ("rststm32", "Reset STM32", "style=\"width:180px\"");
        button_column ("rsteeprom", "Reset EEPROM", "style=\"width:180px\"");
        end_table_row ();
        table_trailer ();
        end_form ();

        if (! strcmp (action, "eepromdump"))
        {
            message = "Currently not implemented";
            // http_eeprom_dump ();
        }

        if (alert_message)
        {
            http_send_FS ("<P><font color=red><B>");
            http_send (alert_message);
            http_send_FS ("</B></font>\r\n");
        }
        else if (message)
        {
            http_send_FS ("<P><font color=green>");
            http_send (message);
            http_send_FS ("</font>\r\n");
        }

        end_box ();
        http_trailer ();
        http_flush ();
    }


    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * network page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_network (void)
{
    const char *                thispage = "network";
    const char *                header_cols[NETWORK_HEADER_COLS] = { "Name", "Value", "Key", "Action" };
    const char *                header_cols2[NETWORK_HEADER_COLS2] = { "Name", "Value", "Action" };
    char *                      action;
    const char *                message = (const char *) 0;
    const char *                alert_message = (const char *) 0;
    char                        alert_buf[80];
    const char *                esp_firmware_version;
    char                        timezone_str[16]; // MAX_TIMEZONE_LEN too small
    int                         networks;
    int                         idx;
    uint_fast16_t               utz;
    int_fast16_t                tz;
    uint_fast8_t                observe_summertime;
    STR_VAR *                   sv;
    uint_fast8_t                rtc             = 0;

    esp_firmware_version        = ESP_VERSION;

    utz = get_numvar (TIMEZONE_NUM_VAR);

    tz = utz & 0xFF;

    if (utz & 0x100)
    {
        tz = -tz;
    }

    if (utz & 0x200)
    {
        observe_summertime = 1;
    }
    else
    {
        observe_summertime = 0;
    }

    action = http_get_param ("action");

    if (*action)
    {
        /* C38 / Review F.6 A4: dieselbe Regel wie die API (EEPROM_*_LEN - 1 Byte, mehr nimmt
         * toCharArray () nicht) - abweisen statt still kuerzen, VOR wifi_connect ().
         */
        if (! strcmp (action, "savewlanlist")
            && (! http_strvar_len_ok (http_get_param ("ssidlist"), EEPROM_SSID_LEN - 1) || ! http_strvar_len_ok (http_get_param ("keylist"), EEPROM_SSID_KEY_LEN - 1)))
        {
            snprintf (alert_buf, sizeof (alert_buf), "SSID max. %u bytes, key max. %u bytes - nothing saved!",
                      (unsigned) (EEPROM_SSID_LEN - 1), (unsigned) (EEPROM_SSID_KEY_LEN - 1));
            alert_message = alert_buf;
        }
        else if (! strcmp (action, "savewlanlist"))
        {
            char * ssid = http_get_param ("ssidlist");
            char * key  = http_get_param ("keylist");

            http_header ("Network", (const char *) NULL, (const char *) NULL);
            http_send_FS ("<B>Connecting to Access Point, try again later.</B>");
            http_trailer ();
            http_flush ();

            wifi_connect (ssid, key, true);

            String pssid = ssid;
            String pkey = key;

            if (! pssid.equals (eeprom_ssid))
            {
                pssid.toCharArray(eeprom_ssid, EEPROM_SSID_LEN);
                eeprom_save_ssid ();
            }
    
            if (! pkey.equals (eeprom_ssidkey))
            {
                pkey.toCharArray (eeprom_ssidkey, EEPROM_SSID_KEY_LEN);
                eeprom_save_ssidkey ();
            }
    
            eeprom_flags &= ~EEPROM_FLAG_BOOT_AS_AP;
            eeprom_save_flags ();
            eeprom_commit ();

            return 0;
        }
        else if (! strcmp (action, "savewlan"))
        {
            char * ssid = http_get_param ("ssid");
            char * key  = http_get_param ("key");

            if (strlen (key) < 10)
            {
                alert_message = "Minimum length of key is 10!";
            }
            else if (! http_strvar_len_ok (ssid, EEPROM_AP_SSID_LEN - 1) || ! http_strvar_len_ok (key, EEPROM_AP_SSID_KEY_LEN - 1))
            {
                snprintf (alert_buf, sizeof (alert_buf), "AP SSID max. %u bytes, key max. %u bytes - nothing saved!",
                          (unsigned) (EEPROM_AP_SSID_LEN - 1), (unsigned) (EEPROM_AP_SSID_KEY_LEN - 1));
                alert_message = alert_buf;                  // C38 / A4
            }
            else
            {
                String  ap_ssid     = ssid;
                String  ap_key      = key;
        
                http_header ("Network", (const char *) NULL, (const char *) NULL);
                http_send_FS ("<B>Restarting as Access Point, try again later.</B>");
                http_trailer ();
                http_flush ();

                if (! ap_ssid.equals (eeprom_ap_ssid))
                {
                    ap_ssid.toCharArray(eeprom_ap_ssid, EEPROM_AP_SSID_LEN);
                    eeprom_save_ap_ssid ();
                }
        
                if (! ap_key.equals (eeprom_ap_ssidkey))
                {
                    ap_key.toCharArray (eeprom_ap_ssidkey, EEPROM_AP_SSID_KEY_LEN);
                    eeprom_save_ap_ssidkey ();
                }
        
                eeprom_flags |= EEPROM_FLAG_BOOT_AS_AP;
                eeprom_save_flags ();
                eeprom_commit ();
        
                wifi_ap (ssid, key);
                return 0;
            }
        }
        else if (! strcmp (action, "savetimeserver"))
        {
            char * newtimeserver = http_get_param ("timeserver");

            set_strvar (TIMESERVER_STR_VAR, newtimeserver);
            message = "Timeserver successfully changed.";
        }
        else if (! strcmp (action, "savetimezone"))
        {
            /* Zweiter, bisher voellig ungepruefter Weg auf TIMEZONE_NUM_VAR neben
             * http_api_network_timezone_set: Ab 256 kippt das Vorzeichenbit 0x100,
             * ab 512 kollidiert der Wert mit dem Sommerzeitbit 0x200 (L28).
             */
            int tz_param = 0;

            if (! http_get_int_param ("timezone", &tz_param) || tz_param < -12 || tz_param > 14)
            {
                alert_message = "Time zone must be a whole number between -12 and 14!";
            }
            else
            {
                tz = tz_param;

                if (get_numvar (TIMEZONE_NUM_VAR) & 0x200)
                {
                    observe_summertime = 1;
                }
                else
                {
                    observe_summertime = 0;
                }

                if (tz < 0)
                {
                    utz = -tz;
                    utz |= 0x100;
                }
                else
                {
                    utz = tz;
                }

                if (observe_summertime)
                {
                    utz |= 0x200;
                }

                set_numvar (TIMEZONE_NUM_VAR, utz);
                message = "Timezone successfully changed.";
            }
        }
        else if (! strcmp (action, "saveobserve_summertime"))
        {
            utz = get_numvar (TIMEZONE_NUM_VAR);

            observe_summertime = http_get_checkbox_param ("observe_summertime");

            if (observe_summertime)
            {
                utz |= 0x200;
            }
            else
            {
                utz &= ~0x200;
            }

            set_numvar (TIMEZONE_NUM_VAR, utz);
            message = "Summertime observation successfully changed.";
        }
        else if (! strcmp (action, "nettime"))
        {
            message = "Getting net time";
            rpc (GET_NET_TIME_RPC_VAR);
        }
        else if (! strcmp (action, "wps"))
        {
            http_header ("Network", (const char *) NULL, (const char *) NULL);
            http_send_FS ("<B>Connecting to AP via WPS, try again later.</B>");
            http_trailer ();
            http_flush ();

            wifi_wps ();
            return 0;
        }
    }

    if (tz >= 0)
    {
        sprintf (timezone_str, "+%d", tz);
    }
    else
    {
        sprintf (timezone_str, "%d", tz);
    }

    http_header ("Network", (const char *) NULL, (const char *) NULL);
    begin_box ("Network");

    http_send_FS ("<table><tr><td>ESP8266 firmware</td><td>");
    http_send (esp_firmware_version);
    http_send_FS ("</td></tr>\r\n");

    http_send_FS ("<tr><td>IP address</td><td>");
    http_send (wifi_ip_address);
    http_send_FS ("</td></tr></table><P>\r\n");

    table_header (header_cols, NETWORK_HEADER_COLS);

    begin_table_row_form (thispage);
    text_column ("WLAN Client");
    http_send_FS ("<td>");
    networks = WiFi.scanNetworks();
    http_send_FS ("<select id=\"ssidlist\" name=\"ssidlist\">");

    for (idx = 0; idx < networks; ++idx)
    {
        String tmp_ssid = WiFi.SSID(idx);

        http_send_FS ("<option value=\"");
        http_send (tmp_ssid);

        if (! strcmp (wifi_ssid, tmp_ssid.c_str()))
        {
            http_send_FS ("\" selected>");
        }
        else
        {
            http_send_FS ("\">");
        }

        http_send (tmp_ssid);
        http_send_FS ("</option>\r\n");
    }

    http_send_FS ("</select></td><td>");
    input_field ("keylist", "", "", MAX_KEY_LEN, 20);
    http_send_FS ("</td>");
    save_column ("wlanlist");
    end_table_row_form ();

    begin_table_row_form (thispage);
    text_column ("WLAN AP");
    http_send_FS ("<td>");
    input_field ("ssid", "", "", WIFI_MAX_SSID_LEN, 20);
    http_send_FS ("</td><td>");
    input_field ("key", "", "", MAX_KEY_LEN, 20);
    http_send_FS ("</td>");
    save_column ("wlan");
    end_table_row_form ();
    table_trailer ();

    begin_form (thispage);
    button_field ("wps", "WPS");
    end_form ();
    http_send_FS ("<P>");

    table_header (header_cols2, NETWORK_HEADER_COLS2);
    sv = get_strvar (TIMESERVER_STR_VAR);
    table_row_input (thispage, 3, "Time server", "timeserver", sv->str, MAX_IP_LEN);
    table_row_input (thispage, 3, "Time zone (GMT +/-)", "timezone", timezone_str, MAX_TIMEZONE_LEN);
    table_row_checkbox (thispage, "Summertime", "observe_summertime", "Observe summertime", observe_summertime);
    table_trailer ();

    begin_form (thispage);
    button_field ("nettime", "Get net time");
    end_form ();
    http_send_FS ("<P>");

    if (alert_message)
    {
        http_send_FS ("<P><font color=red><B>");
        http_send (alert_message);
        http_send_FS ("</B></font>\r\n");
    }
    else if (message)
    {
        http_send_FS ("<P>\r\n<font color=green>");
        http_send (message);
        http_send_FS ("</font>\r\n");
    }

    end_box ();
    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * temperature page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_temperature (void)
{
    const char *    thispage = "temperature";
    const char *    header_cols[MAIN_HEADER_COLS]               = { "Name", "Value" };
    char *          action;
    const char *    message                                     = (const char *) 0;
    uint_fast8_t    rtc                                         = 0;
    char            rtc_temp[16];
    int8_t          rtc_temperature_correction;
    int8_t          temperature_correction;
    char            ds18xx_temp[16];
    uint_fast8_t    temp_index;
    uint_fast8_t    rtc_is_up;

    action = http_get_param ("action");

    if (*action)
    {
        if (! strcmp (action, "savetcorrrtc"))
        {
            int temp_index              = (int) get_numvar (RTC_TEMP_INDEX_NUM_VAR);
            int old_correction          = (int) http_decode_temp_correction (get_numvar (RTC_TEMP_CORRECTION_NUM_VAR));
            int temp_corr               = http_clamp_temp_correction (atoi (http_get_param ("tcorrrtc")));

            rtc_temperature_correction = temp_corr;

            http_rtc_temp_half_deg_correct (rtc_temperature_correction - old_correction);    // A5: Index 49 im selben Schritt
            temp_index -= (rtc_temperature_correction - old_correction);

            if (temp_index < 0)
            {
                temp_index = 0;
            }
            else if (temp_index > 255)
            {
                temp_index = 255;
            }

            set_numvar (RTC_TEMP_INDEX_NUM_VAR, temp_index);
            set_numvar (RTC_TEMP_CORRECTION_NUM_VAR, http_encode_temp_correction (rtc_temperature_correction));
        }
        else if (! strcmp (action, "savetcorrds18xx"))
        {
            int temp_index              = (int) get_numvar (DS18XX_TEMP_INDEX_NUM_VAR);
            int old_correction          = (int) http_decode_temp_correction (get_numvar (DS18XX_TEMP_CORRECTION_NUM_VAR));
            int temp_corr               = http_clamp_temp_correction (atoi (http_get_param ("tcorrds18xx")));

            temperature_correction = temp_corr;

            temp_index -= (temperature_correction - old_correction);

            if (temp_index < 0)
            {
                temp_index = 0;
            }
            else if (temp_index > 255)
            {
                temp_index = 255;
            }

            set_numvar (DS18XX_TEMP_INDEX_NUM_VAR, temp_index);
            set_numvar (DS18XX_TEMP_CORRECTION_NUM_VAR, http_encode_temp_correction (temperature_correction));
        }
        else if (! strcmp (action, "displaytemperature"))
        {
            message = "Displaying temperature...";
            rpc (DISPLAY_TEMPERATURE_RPC_VAR);
        }
    }

    rtc_is_up = get_numvar (RTC_IS_UP_NUM_VAR);

    if (rtc_is_up)
    {
        /* A5 (Ent-8): Minusgrade aus Index 49, Rueckfall auf Index 21 bei 0x8000. Platz: hoechstens
         * "-16383.5" (8 Zeichen) plus "&deg;C" (6) plus Nullbyte, rtc_temp hat 16.
         */
        http_format_half_deg (rtc_temp, sizeof (rtc_temp) - 6, http_rtc_temp_half_deg ());
        strcat (rtc_temp, "&deg;C");
    }
    else
    {
        strcpy (rtc_temp, "offline");
    }

    uint_fast8_t ds18xx_is_up = get_numvar (DS18XX_IS_UP_NUM_VAR);

    if (ds18xx_is_up)
    {
        temp_index = get_numvar (DS18XX_TEMP_INDEX_NUM_VAR);
        sprintf (ds18xx_temp, "%d", temp_index / 2);

        if (temp_index % 2)
        {
            strcat (ds18xx_temp, ".5");
        }

        strcat (ds18xx_temp, "&deg;C");
    }
    else
    {
        strcpy (ds18xx_temp, "offline");
    }

    rtc_temperature_correction = http_decode_temp_correction (get_numvar (RTC_TEMP_CORRECTION_NUM_VAR));
    temperature_correction = http_decode_temp_correction (get_numvar (DS18XX_TEMP_CORRECTION_NUM_VAR));

    http_header ("Temperature", (const char *) NULL, (const char *) NULL);
    begin_box ("Temperature");

    table_header (header_cols, MAIN_HEADER_COLS);
    table_row ("RTC temperature", rtc_temp, "");
    table_row ("DS18xx", ds18xx_temp, "");
    table_row_input (thispage, 3, "Temp correction RTC (units of 0.5&deg;C)", "tcorrrtc", rtc_temperature_correction, MAX_TEMP_CORR_LEN);
    table_row_input (thispage, 3, "Temp correction DS18xx (units of 0.5&deg;C)", "tcorrds18xx", temperature_correction, MAX_TEMP_CORR_LEN);
    table_trailer ();

    begin_form (thispage);
    button_field ("displaytemperature", "Display temperature");
    end_form ();

    if (message)
    {
        http_send_FS ("<P><font color=green>");
        http_send (message);
        http_send_FS ("</font>\r\n");
    }

    end_box ();
    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * weather page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_weather (void)
{
    const char *                thispage = "weather";
    const char *                header_cols[WEATHER_HEADER_COLS] = { "Name", "Value", "Action" };
    char *                      action;
    const char *                message = (const char *) 0;
    const char *                alert_message = (const char *) 0;
    STR_VAR *                   sv;
    uint_fast8_t                rtc     = 0;

    action = http_get_param ("action");

    if (*action)
    {
        if (! strcmp (action, "saveappid"))
        {
            char * newappid = http_get_param ("appid");

            set_strvar (WEATHER_APPID_STR_VAR, newappid);
            message = "AppID successfully changed.";
        }
        else if (! strcmp (action, "savecity"))
        {
            char * newcity = http_get_param ("city");

            set_strvar (WEATHER_CITY_STR_VAR, newcity);
            message = "City successfully changed.";
        }
        else if (! strcmp (action, "savelonlat"))
        {
            char * newlon = http_get_param ("lon");
            char * newlat = http_get_param ("lat");

            strsubst (newlon, ',', '.');
            strsubst (newlat, ',', '.');

            set_strvar (WEATHER_LON_STR_VAR, newlon);
            set_strvar (WEATHER_LAT_STR_VAR, newlat);
            message = "Coordinates successfully changed.";
        }
        else if (! strcmp (action, "getweather"))
        {
            message = "Getting weather";
            rpc (GET_WEATHER_RPC_VAR);
        }
        else if (! strcmp (action, "getweatherfc"))
        {
            message = "Getting weather forecast";
            rpc (GET_WEATHER_FC_RPC_VAR);
        }
    }

    http_header ("Weather", (const char *) NULL, (const char *) NULL);
    begin_box ("Weather");

    table_header (header_cols, WEATHER_HEADER_COLS);

    sv = get_strvar (WEATHER_APPID_STR_VAR);
    table_row_input (thispage, 3, "APPID", "appid", sv->str, MAX_WEATHER_APPID_LEN);
    sv = get_strvar (WEATHER_CITY_STR_VAR);
    table_row_input (thispage, 3, "City", "city", sv->str, MAX_WEATHER_CITY_LEN);

    const char *    ids[2]                                      = { "lon", "lat" };
    const char *    desc[2]                                     = { "LON", "LAT" };
    int             maxlen[2]                                   = { MAX_WEATHER_LON_LEN, MAX_WEATHER_LAT_LEN };
    int             maxsize[2]                                  = { MAX_WEATHER_LON_LEN, MAX_WEATHER_LAT_LEN };
    const char *    values[2];

    sv = get_strvar (WEATHER_LON_STR_VAR);
    values[0] = sv->str;
    sv = get_strvar (WEATHER_LAT_STR_VAR);
    values[1] = sv->str;

    table_row_inputs (thispage, "lonlat", 2, ids, desc, values, maxlen, maxsize);

    table_trailer ();

    begin_form (thispage);
    button_field ("getweather", "Get weather");
    button_field ("getweatherfc", "Get weather forecast");
    end_form ();

    if (alert_message)
    {
        http_send_FS ("<P><font color=red><B>");
        http_send (alert_message);
        http_send_FS ("</B></font>\r\n");
    }
    else if (message)
    {
        http_send_FS ("<P><font color=green>");
        http_send (message);
        http_send_FS ("</font>\r\n");
    }

    end_box ();
    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * ldr page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_ldr (void)
{
    const char *    thispage = "ldr";
    const char *    header_cols[MAIN_HEADER_COLS]               = { "Name", "Value" };
    char *          action;
    const char *    message                                     = (char *) 0;
    uint_fast8_t    rtc                                         = 0;
    uint16_t        raw_value;
    uint16_t        min_value;
    uint16_t        max_value;
    uint_fast8_t    auto_brightness_active;
    char            raw_buf[MAX_RAW_VALUE_LEN + 1];
    char            min_buf[MAX_RAW_VALUE_LEN + 1];
    char            max_buf[MAX_RAW_VALUE_LEN + 1];

    auto_brightness_active  = get_numvar (DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE_NUM_VAR);

    action = http_get_param ("action");

    if (*action)
    {
        if (! strcmp (action, "ldrmin"))
        {
            rpc (LDR_MIN_VALUE_RPC_VAR);
            message = "Stored minimum value";
        }
        else if (! strcmp (action, "ldrmax"))
        {
            rpc (LDR_MAX_VALUE_RPC_VAR);
            message = "Stored maximum value";
        }
        else if (! strcmp (action, "saveauto"))
        {
            if (http_get_checkbox_param ("auto"))
            {
                auto_brightness_active = 1;
            }
            else
            {
                auto_brightness_active = 0;
            }

            set_numvar (DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE_NUM_VAR, auto_brightness_active);
        }
    }

    http_header ("LDR", (const char *) NULL, (const char *) NULL);
    begin_box ("LDR");

    table_header (header_cols, MAIN_HEADER_COLS);

    if (auto_brightness_active)
    {
        raw_value = get_numvar (LDR_RAW_VALUE_NUM_VAR);
        min_value = get_numvar (LDR_MIN_VALUE_NUM_VAR);
        max_value = get_numvar (LDR_MAX_VALUE_NUM_VAR);

        sprintf (raw_buf, "%u", raw_value);
        sprintf (min_buf, "%u", min_value);
        sprintf (max_buf, "%u", max_value);

        table_row ("LDR", raw_buf, "");
        table_row ("Min", min_buf, "");
        table_row ("Max", max_buf, "");
    }

    table_row_checkbox (thispage, "LDR", "auto", "Automatic brightness", auto_brightness_active);

    table_trailer ();

    if (auto_brightness_active)
    {
        begin_form (thispage);
        button_field ("ldrmin", "Set as minimum value");
        button_field ("ldrmax", "Set as maximum value");
        end_form ();
    }

    if (message)
    {
        http_send_FS ("<P><font color=green>");
        http_send (message);
        http_send_FS ("</font>\r\n");
    }

    end_box ();
    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display brightness page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_display_brightness (void)
{
    const char *    thispage = "dispbright";
    const char *    header_cols[MAIN_HEADER_COLS]               = { "Brightness", "Value" };
    char            txtbuf[32];
    char            idbuf[32];
    char            valbuf[32];
    uint_fast8_t    val;
    char *          action;
    int             idx;
    uint_fast8_t    rtc                                         = 0;

    action = http_get_param ("action");

    if (*action)
    {
        if (! strncmp (action, "savedim", 7))
        {
            idx = atoi (action + 7);
            val = atoi (http_get_param (action + 4));
            set_num8_array (DISPLAY_DIMMED_DISPLAY_COLORS, idx, val);
        }
    }

    http_header ("Brightness", (const char *) NULL, (const char *) NULL);
    begin_box ("Brightness");

    table_header (header_cols, MAIN_HEADER_COLS);

    for (idx = 0; idx < MAX_BRIGHTNESS + 1; idx++)         // from 0 to 15 = 16 values
    {
        val = get_num8_array (DISPLAY_DIMMED_DISPLAY_COLORS, idx);

        sprintf (txtbuf, "%2d", idx);
        sprintf (idbuf, "dim%d", idx);
        sprintf (valbuf, "%d", val);
        table_row_slider (thispage, txtbuf, idbuf, valbuf, "0", "15");
    }

    table_trailer ();

    end_box ();
    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * ambilight brightness page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_ambilight_brightness (void)
{
    const char *    thispage = "ambibright";
    const char *    header_cols[MAIN_HEADER_COLS]               = { "Brightness", "Value" };
    char            txtbuf[32];
    char            idbuf[32];
    char            valbuf[32];
    uint_fast8_t    val;
    char *          action;
    int             idx;
    uint_fast8_t    rtc                                         = 0;

    action = http_get_param ("action");

    if (*action)
    {
        if (! strncmp (action, "savedim", 7))
        {
            idx = atoi (action + 7);
            val = atoi (http_get_param (action + 4));
            set_num8_array (DISPLAY_DIMMED_AMBILIGHT_COLORS, idx, val);
        }
    }

    http_header ("Ambilight Brightness", (const char *) NULL, (const char *) NULL);
    begin_box ("Ambilight Brightness");

    table_header (header_cols, MAIN_HEADER_COLS);

    for (idx = 0; idx < MAX_BRIGHTNESS + 1; idx++)         // from 0 to 15 = 16 values
    {
        val = get_num8_array (DISPLAY_DIMMED_AMBILIGHT_COLORS, idx);

        sprintf (txtbuf, "%2d", idx);
        sprintf (idbuf, "dim%d", idx);
        sprintf (valbuf, "%d", val);
        table_row_slider (thispage, txtbuf, idbuf, valbuf, "0", "15");
    }

    table_trailer ();

    end_box ();
    http_trailer ();
    http_flush ();

    return rtc;
}

static void
select_icons (const char * id, const char * name)
{
    static const char * const icon_candidates[] = { "wc24h-icon.txt", "wc12h-icon.txt", "uc-icon.txt" };
    const char *  fname = (const char *) 0;
    char          icon_name[32 + 1];

    http_send_FS ("<select id=\"");
    http_send (id);
    http_send_FS ("\" name=\"");
    http_send (id);
    http_send_FS ("\">");

    LittleFS.begin ();

    fname = http_find_existing_filename (http_get_configured_icon_filename (), icon_candidates, sizeof (icon_candidates) / sizeof (icon_candidates[0]));

    if (fname)
    {
        File    fp;

        fp = LittleFS.open (fname, "r");
    
        if (fp)
        {
            int     ch;
            int     i;

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
        
                http_send_FS ("<option value=\"");
                http_send (icon_name);
                http_send_FS ("\"");
        
                if (! strcmp (name, icon_name))
                {
                    http_send_FS (" selected");
                }
        
                http_send_FS (">");
                http_send (icon_name);
                http_send_FS ("</option>\r\n");
        
                while ((ch = fp.read()) != '*' && ch >= 0)
                {
                    ;
                }
            }
    
            fp.close ();
        }
    }

    LittleFS.end ();

    http_send_FS ("</select>\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MAX_DISPLAY_MODE_VARIABLES  24
static uint_fast8_t
http_display (void)
{
    const char *        thispage = "display";
    const char *        header_cols[DISPLAY_HEADER_COLS]  = { "Name", "Value", "Action" };
    char *              action;
    const char *        message         = (const char *) 0;
    uint_fast8_t        use_rgbw        = get_numvar (DISPLAY_USE_RGBW_NUM_VAR);
    DSP_COLORS          rgbw;
    const char *        ids[4]          = { "red", "green", "blue", "white" };
    const char *        desc[4]         = { "R", "G", "B", "W" };
    char *              rgbw_buf[4];
    const char *        minval[4]       = { "0",   "0",  "0",  "0" };
    const char *        maxval[4]       = { "63", "63", "63", "63" };
    const char *        display_mode_names[MAX_DISPLAY_MODE_VARIABLES];
    int                 color_animation_mode;
    uint_fast8_t        auto_brightness_active;
    uint_fast8_t        display_brightness;
    char                brbuf[MAX_BRIGHTNESS_LEN + 1];
    char                red_buf[MAX_COLOR_VALUE_LEN + 16];
    char                green_buf[MAX_COLOR_VALUE_LEN + 16];
    char                blue_buf[MAX_COLOR_VALUE_LEN + 16];
    char                white_buf[MAX_COLOR_VALUE_LEN + 16];
    int                 display_mode;
    STR_VAR *           sv;
    uint_fast8_t        idx;
    uint_fast8_t        display_flags;
    uint_fast8_t        permanent_display_of_it_is;
    uint_fast8_t        ticker_deceleration;
    uint_fast8_t        rtc = 0;

    display_mode                    = get_numvar (DISPLAY_MODE_NUM_VAR);
    display_flags                   = get_numvar (DISPLAY_FLAGS_NUM_VAR);
    color_animation_mode            = get_numvar (COLOR_ANIMATION_MODE_NUM_VAR);
    display_brightness              = get_numvar (DISPLAY_BRIGHTNESS_NUM_VAR);
    auto_brightness_active          = get_numvar (DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE_NUM_VAR);
    ticker_deceleration             = get_numvar (TICKER_DECELRATION_NUM_VAR);

    get_dsp_color_var (DISPLAY_DSP_COLOR_VAR, &rgbw);

    permanent_display_of_it_is      = (display_flags & DISPLAY_FLAGS_PERMANENT_IT_IS) ? 1 : 0;

    for (idx = 0; idx < display_modes_count; idx++)
    {
        display_mode_names[idx] = tbl_modes[idx].description;
    }

    action = http_get_param ("action");

    if (*action)
    {
        if (! strcmp (action, "saveitis"))
        {
            if (http_get_checkbox_param ("itis"))
            {
                permanent_display_of_it_is = 1;
                display_flags |= DISPLAY_FLAGS_PERMANENT_IT_IS;
            }
            else
            {
                permanent_display_of_it_is = 0;
                display_flags &= ~DISPLAY_FLAGS_PERMANENT_IT_IS;
            }

            set_numvar (DISPLAY_FLAGS_NUM_VAR, display_flags);
        }
        else if (! strcmp (action, "savebrightness"))
        {
            display_brightness = atoi (http_get_param ("brightness"));
            set_numvar (DISPLAY_BRIGHTNESS_NUM_VAR, display_brightness);
        }
        else if (! strcmp (action, "savecolors"))
        {
            rgbw.red     = atoi (http_get_param ("red"));
            rgbw.green   = atoi (http_get_param ("green"));
            rgbw.blue    = atoi (http_get_param ("blue"));

            if (use_rgbw)
            {
                rgbw.white = atoi (http_get_param ("white"));
            }
            else
            {
                rgbw.white = 0;
            }

            set_dsp_color_var (DISPLAY_DSP_COLOR_VAR, &rgbw, use_rgbw);
        }
        else if (! strcmp (action, "savedisplaymode"))
        {
            display_mode = atoi (http_get_param ("displaymode"));
            set_numvar (DISPLAY_MODE_NUM_VAR, display_mode);
        }
        else if (! strcmp (action, "savetickerdec"))
        {
            ticker_deceleration = atoi (http_get_param ("tickerdec"));
            set_numvar (TICKER_DECELRATION_NUM_VAR, ticker_deceleration);
        }
        else if (! strcmp (action, "savedtf"))
        {
            char * date_ticker_format = http_get_param ("dtf");

            set_strvar (DATE_TICKER_FORMAT_VAR, date_ticker_format);
            message = "date ticker format successfully changed.";
        }
        else if (! strcmp (action, "testdisplay"))
        {
            message = "Testing display...";
            rpc (TEST_DISPLAY_RPC_VAR);
        }
        else if (! strcmp (action, "poweron"))
        {
            message = "Switching power on...";
            set_numvar (DISPLAY_POWER_NUM_VAR, 1);
        }
        else if (! strcmp (action, "poweroff"))
        {
            message = "Switching power off...";
            set_numvar (DISPLAY_POWER_NUM_VAR, 0);
        }
        else if (! strcmp (action, "apoweron"))
        {
            message = "Switching ambilight power on...";
            set_numvar (DISPLAY_AMBILIGHT_POWER_NUM_VAR, 1);
        }
        else if (! strcmp (action, "apoweroff"))
        {
            message = "Switching ambilight power off...";
            set_numvar (DISPLAY_AMBILIGHT_POWER_NUM_VAR, 0);
        }
    }

    sprintf (brbuf,     "%d", display_brightness);
    sprintf (red_buf,   "%d", rgbw.red);
    sprintf (green_buf, "%d", rgbw.green);
    sprintf (blue_buf,  "%d", rgbw.blue);

    if (use_rgbw)
    {
        sprintf (white_buf, "%d", rgbw.white);
    }
    else
    {
        white_buf[0] = '0';
        white_buf[1] = '\0';
    }

    rgbw_buf[0] = red_buf;
    rgbw_buf[1] = green_buf;
    rgbw_buf[2] = blue_buf;
    rgbw_buf[3] = white_buf;

    http_header ("Display", (const char *) NULL, (const char *) NULL);
    begin_box ("Display");
    message_tables_file_missing ();

    table_header (header_cols, DISPLAY_HEADER_COLS);

    table_row_checkbox (thispage, "ES IST", "itis", "Permanent display of \"ES IST\"", permanent_display_of_it_is);
    table_row_select (thispage, "Display Mode", "displaymode", display_mode_names, display_mode, 0, display_modes_count, 0);

    if (! auto_brightness_active)
    {
        table_row_slider (thispage, "Brightness (0-15)", "brightness", brbuf, "0", "15");
    }

    if (color_animation_mode == COLOR_ANIMATION_MODE_NONE)
    {
        uint_fast8_t    n_colors;

        if (use_rgbw)
        {
            n_colors = 4;
        }
        else
        {
            n_colors = 3;
        }

        table_row_sliders (thispage, "Colors", "colors", n_colors, ids, desc, rgbw_buf, minval, maxval);
    }

    table_row_input (thispage, 3, "Ticker deceleration", "tickerdec", ticker_deceleration, MAX_TICKER_DECELERATION_LEN);

    sv = get_strvar (DATE_TICKER_FORMAT_VAR);
    table_row_input (thispage, 3, "Date ticker format", "dtf", sv->str, MAX_DATE_TICKER_FORMAT_LEN);

    table_trailer ();

    begin_form (thispage);
    button_field ("poweron", "Power On");
    button_field ("poweroff", "Power Off");
    button_field ("apoweron", "Ambilight Power On");
    button_field ("apoweroff", "Ambilight Power Off");
    button_field ("testdisplay", "Test display");
    end_form ();

    if (message)
    {
        http_send_FS ("<P><font color=green>");
        http_send (message);
        http_send_FS ("</font>\r\n");
    }

    end_box ();
    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animations page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_animations (void)
{
    const char *        thispage = "animations";
    const char *        header_cols[ANIMATION_HEADER_COLS] = { "Name", "Value", "Action" };
    const char *        dec_cols[ANIMATION_DECELERATION_HEADER_COLS] = { "Name", "Deceleration", "Default", "Favourite", "Action" };
    const char *        color_dec_cols[COLOR_ANIMATION_DECELERATION_HEADER_COLS] = { "Name", "Deceleration", "Default", "Action" };
    static const char * animation_mode_names[MAX_DISPLAY_ANIMATION_VARIABLES];
    static const char * color_animation_mode_names[MAX_COLOR_ANIMATION_VARIABLES];
    static int          already_called;
    const char *        message         = (const char *) 0;
    char *              action;
    unsigned int        animation_mode;
    int                 color_animation_mode;
    char                animidbuf[8];
    char                decidbuf[8];
    char                defaultidbuf[8];
    char                favidbuf[8];
    char                color_animidbuf[8];
    char                color_decidbuf[8];
    char                color_defaultidbuf[8];
    char                color_decbuf[MAX_COLOR_ANIMATION_DECELERATION_LEN + 1];

    char                decbuf[MAX_ANIMATION_DECELERATION_LEN + 1];
    uint_fast8_t        idx;
    uint_fast8_t        rtc = 0;

    if (! already_called)
    {
        for (idx = 0; idx < max_display_animation_variables; idx++)
        {
            DISPLAY_ANIMATION * da = get_display_animation_var (idx);
            animation_mode_names[idx] = da->name;
        }

        for (idx = 0; idx < MAX_COLOR_ANIMATION_VARIABLES; idx++)
        {
            COLOR_ANIMATION * ca = get_color_animation_var ((COLOR_ANIMATION_VARIABLE) idx);
            color_animation_mode_names[idx] = ca->name;
        }

        already_called = 1;
    }

    animation_mode          = get_numvar (ANIMATION_MODE_NUM_VAR);
    color_animation_mode    = get_numvar (COLOR_ANIMATION_MODE_NUM_VAR);

    action = http_get_param ("action");

    if (*action)
    {
        if (! strcmp (action, "saveanimation"))
        {
            animation_mode = atoi (http_get_param ("animation"));
            set_numvar (ANIMATION_MODE_NUM_VAR, animation_mode);
        }
        else if (! strcmp (action, "savecoloranimation"))
        {
            color_animation_mode = atoi (http_get_param ("coloranimation"));
            set_numvar (COLOR_ANIMATION_MODE_NUM_VAR, color_animation_mode);
        }

        else if (! strncmp (action, "def", 3))
        {
            uint_fast8_t    animation_idx;

            animation_idx = atoi (action + 3);

            if (animation_idx < max_display_animation_variables)
            {
                DISPLAY_ANIMATION * da = get_display_animation_var (animation_idx);
                set_display_animation_deceleration (animation_idx, da->default_deceleration);
                set_display_animation_flags (animation_idx, da->flags | ANIMATION_FLAG_FAVOURITE);
            }
        }
        else if (! strncmp (action, "savean", 6))
        {
            uint_fast8_t    animation_idx;
            uint_fast8_t    animation_deceleration;
            uint_fast8_t    animation_favourite;
            animation_idx = atoi (action + 6);

            if (animation_idx < max_display_animation_variables)
            {
                DISPLAY_ANIMATION * da = get_display_animation_var (animation_idx);
                sprintf (decidbuf, "sp%d", animation_idx);
                sprintf (favidbuf, "fav%d", animation_idx);

                animation_deceleration = atoi (http_get_param (decidbuf));

                if (animation_deceleration >= ANIMATION_MIN_DECELERATION && animation_deceleration <= ANIMATION_MAX_DECELERATION)
                {
                    set_display_animation_deceleration (animation_idx, animation_deceleration);
                }

                animation_favourite = http_get_checkbox_param (favidbuf);

                if (animation_favourite)
                {
                    set_display_animation_flags (animation_idx, da->flags | ANIMATION_FLAG_FAVOURITE);
                }
                else
                {
                    set_display_animation_flags (animation_idx, da->flags & ~ANIMATION_FLAG_FAVOURITE);
                }
            }
        }
        else if (! strncmp (action, "savecan", 7))
        {
            uint_fast8_t    color_animation_idx;
            uint_fast8_t    color_animation_deceleration;

            color_animation_idx = atoi (action + 7);

            if (color_animation_idx < MAX_COLOR_ANIMATION_VARIABLES)
            {
                sprintf (color_decidbuf, "csp%d", color_animation_idx);

                color_animation_deceleration = atoi (http_get_param (color_decidbuf));

                if (color_animation_deceleration <= COLOR_ANIMATION_MAX_DECELERATION)
                {
                    set_color_animation_deceleration ((COLOR_ANIMATION_VARIABLE) color_animation_idx, color_animation_deceleration);
                }
            }
        }
        else if (! strncmp (action, "cdef", 4))
        {
            COLOR_ANIMATION_VARIABLE    color_animation_idx;

            color_animation_idx = (COLOR_ANIMATION_VARIABLE) atoi (action + 4);

            if (color_animation_idx < MAX_COLOR_ANIMATION_VARIABLES)
            {
                COLOR_ANIMATION * ca = get_color_animation_var (color_animation_idx);
                set_color_animation_deceleration (color_animation_idx, ca->default_deceleration);
            }
        }
    }

    http_header ("Animations", (const char *) NULL, (const char *) NULL);
    begin_box ("Animations");

    table_header (header_cols, ANIMATION_HEADER_COLS);
    table_row_select (thispage, "Animation", "animation", animation_mode_names, animation_mode, 0, max_display_animation_variables, 0);
    table_row_select (thispage, "Color Animation", "coloranimation", color_animation_mode_names, color_animation_mode, 0, MAX_COLOR_ANIMATION_VARIABLES, 0);
    table_trailer ();

    table_header (dec_cols, ANIMATION_DECELERATION_HEADER_COLS);

    for (idx = 0; idx < max_display_animation_variables; idx++)
    {
        DISPLAY_ANIMATION * da = get_display_animation_var (idx);

        if (da->flags & ANIMATION_FLAG_CONFIGURABLE)
        {
            sprintf (animidbuf, "an%d", idx);
            sprintf (decidbuf, "sp%d", idx);
            sprintf (defaultidbuf, "def%d", idx);
            sprintf (favidbuf, "fav%d", idx);
            sprintf (decbuf, "%d", da->deceleration);

            begin_table_row_form (thispage);
            text_column (da->name);
            slider_column (decidbuf, decbuf, "1", "15");
            button_column (defaultidbuf, "Default");
            checkbox_column (favidbuf, "", (da->flags & ANIMATION_FLAG_FAVOURITE) ? 1 : 0);
            save_column (animidbuf);
            end_table_row_form ();
        }
    }

    table_trailer ();

    table_header (color_dec_cols, COLOR_ANIMATION_DECELERATION_HEADER_COLS);

    for (idx = 0; idx < MAX_COLOR_ANIMATION_VARIABLES; idx++)
    {
        COLOR_ANIMATION * color_animation;
        color_animation = get_color_animation_var ((COLOR_ANIMATION_VARIABLE) idx);

        if (color_animation->flags & COLOR_ANIMATION_FLAG_CONFIGURABLE)
        {
            sprintf (color_animidbuf, "can%d", idx);
            sprintf (color_decidbuf, "csp%d", idx);
            sprintf (color_defaultidbuf, "cdef%d", idx);
            sprintf (color_decbuf, "%d", color_animation->deceleration);

            begin_table_row_form (thispage);
            text_column (color_animation->name);
            slider_column (color_decidbuf, color_decbuf, "0", "15");
            button_column (color_defaultidbuf, "Default");
            save_column (color_animidbuf);
            end_table_row_form ();
        }
    }

    table_trailer ();


    begin_form (thispage);
    end_form ();

    if (message)
    {
        http_send_FS ("<P><font color=green>");
        http_send (message);
        http_send_FS ("</font>\r\n");
    }

    end_box ();
    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * overlays page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define N_OVERLAY_TYPES         11
#define N_DATE_CODES            7
#define OVERLAY_MAX_FOLDER_LEN  2
#define OVERLAY_MAX_TRACK_LEN   3

const char * overlay_types[N_OVERLAY_TYPES] =
{
    "----",
    "Icon",
    "Date",
    "Temperature",
    "Weather Icon",
    "Weather Ticker",
    "Ticker",
    "DFPlayer",
    "Weather FC Icon",
    "Weather FC Ticker",
    "Temperature as Digits",
};

/* C6u: type, date_code und days des Legacy-Overlayformulars kamen per blankem atoi herein -
 * "abc" wurde 0, type 99 und date_code 300 gingen unbesehen an den STM, days 0 still zu 1.
 * Jetzt dieselbe Regel wie in http_api_overlay_set () und derselbe Wortlaut wie C18 ("<feld>
 * out of range (lo..hi)"): type muss stehen, date_code und days duerfen fehlen - das Formular
 * "New overlay" schickt nur otype, eine Zeile ohne Datum schickt kein od. Liefert 1, wenn alles
 * passt; sonst 0 und die Meldung in msg. Es wird NICHTS geschrieben - das ist Sache des Aufrufers.
 */
static uint_fast8_t
http_overlays_get_fields (int * typep, int * date_codep, int * daysp, char * msg, size_t msg_len)
{
    *date_codep = OVERLAY_DATE_CODE_NONE;
    *daysp      = 1;

    if (! http_get_int_param ("otype", typep) || *typep < OVERLAY_TYPE_NONE || *typep > OVERLAY_TYPE_TEMPERATURE_DIGITS)
    {
        snprintf (msg, msg_len, "type out of range (%d..%d), nothing saved", OVERLAY_TYPE_NONE, OVERLAY_TYPE_TEMPERATURE_DIGITS);
        return 0;
    }

    if (! http_get_opt_int_param ("odc", date_codep, OVERLAY_DATE_CODE_NONE, OVERLAY_DATE_CODE_ADVENT4))
    {
        snprintf (msg, msg_len, "date_code out of range (%d..%d), nothing saved", OVERLAY_DATE_CODE_NONE, OVERLAY_DATE_CODE_ADVENT4);
        return 0;
    }

    if (! http_get_opt_int_param ("od", daysp, 1, 255))
    {
        snprintf (msg, msg_len, "days out of range (1..255), nothing saved");
        return 0;
    }

    return 1;
}

static uint_fast8_t
http_overlays (void)
{
    const char *        thispage = "overlays";
    const char *        overlay_cols[OVERLAY_HEADER_COLS] = { "Active", "Type", "Value", "Interval<BR>(min)", "Duration<BR>(sec)",
                                                              "Date<BR>Code", "or", "MM", "DD", "Days" };
    const char *        date_codes[N_DATE_CODES] = { "----", "Carnival", "Easter Sunday", "1st Advent", "2nd Advent", "3rd Advent", "4th Advent" };
    char *              action;
    const char *        message         = (const char *) 0;
    uint_fast8_t        overlay_idx;
    uint_fast8_t        n_overlays;
    char                id[8];
    char                disp[8];
    char                hidden[64];
    uint_fast8_t        idx;
    uint_fast8_t        oidx = 0xFF;
    uint_fast8_t        rtc = 0;
    int                 otype = OVERLAY_TYPE_NONE;
    int                 odc = OVERLAY_DATE_CODE_NONE;
    int                 odays = 1;
    char                msgbuf[64];

    action = http_get_param ("action");

    if (! *action)
    {
        action = http_get_param ("oid");       // id by autosubmit
    }

    n_overlays = get_numvar (OVERLAY_N_OVERLAYS_NUM_VAR);

    if (*action)
    {
        if (! strncmp (action, "disp", 4))
        {
            overlay_idx = atoi (action + 4);

            /* S.5: auch hier ungeprueft gewesen. Kein Schreibzugriff auf overlays[], aber der Index ging
             * unbesehen an den STM; dort faengt ihn nur main.c (show_overlay_idx < MAX_OVERLAYS) ab.
             * Die Liste bietet ohnehin nur belegte Plaetze an.
             */
            if (overlay_idx < n_overlays && overlay_idx < MAX_OVERLAYS)
            {
                set_numvar (DISPLAY_OVERLAY_NUM_VAR, overlay_idx);
            }
            else
            {
                message = "Invalid overlay index, nothing displayed";
            }
        }
        else if (! strncmp (action, "saveoid", 7))
        {
            oidx = atoi (action + 7);
        }
        else if (! strncmp (action, "oid", 3))
        {
            oidx = atoi (action + 3);
        }

        /* N1 (S.5, design 2.12): oidx kommt per atoi aus der URL -- auch per <img> --, und uint_fast8_t
         * ist auf dem ESP 32 Bit breit: "oid40" schrieb overlays[40], "oid-1" overlays[4294967295].
         * Geprueft wird VOR jedem Schreibzugriff und vor n_overlays++. oidx == n_overlays haengt an,
         * eine Luecke dahinter nicht -- dieselbe Regel wie http_api_overlay_set().
         */
        if (oidx != 0xff && (oidx >= MAX_OVERLAYS || oidx > n_overlays))
        {
            message = "Invalid overlay index, nothing saved";
        }
        else if (oidx != 0xff && ! http_overlays_get_fields (&otype, &odc, &odays, msgbuf, sizeof (msgbuf)))
        {
            message = msgbuf;                                                   // C6u: nach dem Indexguard, vor n_overlays++
        }
        else if (oidx != 0xff)
        {
            uint_fast8_t  val;
            uint_fast8_t  valmm;
            uint_fast8_t  valdd;

            if (oidx == n_overlays)
            {
                n_overlays++;
                set_numvar (OVERLAY_N_OVERLAYS_NUM_VAR, n_overlays);
            }

            if (http_get_checkbox_param ("oact"))
            {
                overlays[oidx].flags |= OVERLAY_FLAG_ACTIVE;
            }
            else
            {
                overlays[oidx].flags &= ~OVERLAY_FLAG_ACTIVE;
            }
            
            overlays[oidx].type = otype;
            val = atoi (http_get_param ("oint"));

            if (val == 0)
            {
                val = 5;
            }

            overlays[oidx].interval = val;

            val = atoi (http_get_param ("oduration"));

            if (val < 5)
            {
                val = 5;
            }
            else if (val > 9)
            {
                val = 9;
            }

            overlays[oidx].duration = val;

            overlays[oidx].date_code   = odc;

            valmm = atoi (http_get_param ("odstmm"));
            valdd = atoi (http_get_param ("odstdd"));

            if (valmm < 1 || valmm > 12 || valdd < 1 || valdd > 31)
            {
                overlays[oidx].date_start = 0;
            }
            else
            {
                overlays[oidx].date_start  = (valmm << 8) | valdd;
            }

            overlays[oidx].days = odays;

            if (overlays[oidx].type == OVERLAY_TYPE_MP3)
            {
                sprintf (overlays[oidx].text, "%02d/%03d", atoi (http_get_param ("ofo")), atoi (http_get_param ("otr")));
            }
            else
            {
                /* Auch der Legacy-Weg darf kein halbes Zeichen hinterlassen - die
                 * settings_xml liest die PWA, nicht Legacy (L46).
                 */
                utf8_copy_truncated (overlays[oidx].text, http_get_param ("oname"), OVERLAY_MAX_TEXT_LEN);
            }

            set_overlay_var (oidx);
        }
    }

    http_header ("Overlays", (const char *) NULL, (const char *) NULL);
    begin_box ("Overlays");
    message_icon_files_missing ();
    message_tables_file_missing ();

    n_overlays = http_n_overlays_for_read ();                                   // S.5: Lesen begrenzen

    if (n_overlays > 0)
    {
        table_header (overlay_cols, OVERLAY_HEADER_COLS);

        for (idx = 0; idx < n_overlays; idx++)
        {
            sprintf (disp, "%d", idx);
            sprintf (id, "oid%d", idx);
            sprintf (hidden, "<input type=\"hidden\" name=\"oid\" value=\"oid%d\">", idx);

            begin_table_row_form (thispage);

            checkbox_column ("oact", "", (overlays[idx].flags & OVERLAY_FLAG_ACTIVE) ? 1 : 0);
            select_column ("otype", overlay_types, overlays[idx].type, 0, N_OVERLAY_TYPES, 1);

            if (overlays[idx].type == OVERLAY_TYPE_ICON)
            {
                begin_column ();
                select_icons ("oname", overlays[idx].text);
                end_column ();
            }
            else if (overlays[idx].type == OVERLAY_TYPE_TICKER)
            {
                input_column ("oname", "", overlays[idx].text, OVERLAY_MAX_TEXT_LEN, OVERLAY_MAX_TEXT_LEN / 2);
            }
            else if (overlays[idx].type == OVERLAY_TYPE_MP3)
            {
                char    folderbuf[8];
                char    trackbuf[8];
                char *  p;

                p = strchr (overlays[idx].text, '/');

                if (p)
                {
                    int   folder;
                    int   track;

                    folder = atoi (overlays[idx].text);
                    track  = atoi (p + 1);
                    sprintf (folderbuf, "%02d", folder);
                    sprintf (trackbuf, "%03d", track);
                }
                else
                {
                    folderbuf[0] = '\0';
                    trackbuf[0] = '\0';
                }

                begin_column ();
                input_field ("ofo", "F", folderbuf, OVERLAY_MAX_FOLDER_LEN, OVERLAY_MAX_FOLDER_LEN);
                input_field ("otr", "T",  trackbuf, OVERLAY_MAX_TRACK_LEN,  OVERLAY_MAX_TRACK_LEN);
                end_column ();
            }
            else
            {
                text_column ("");
            }

            input_column ("oint",  "", overlays[idx].interval, 2, 2);

            if (overlays[idx].type == OVERLAY_TYPE_ICON || overlays[idx].type == OVERLAY_TYPE_WEATHER_ICON || overlays[idx].type == OVERLAY_TYPE_WEATHER_FC_ICON)
            {
                input_column ("oduration",  "", overlays[idx].duration, 1, 1);
            }
            else
            {
                 text_column ("");
            }

            select_column ("odc", date_codes, overlays[idx].date_code, 0, N_DATE_CODES, 1);

            text_column (hidden);     // "or"

            if (overlays[idx].date_code == OVERLAY_DATE_CODE_NONE)
            {
                if (overlays[idx].date_start == 0)
                {
                    input_column ("odstmm",     "", "", 2, 2);
                    input_column ("odstdd",     "", "", 2, 2);
                }
                else
                {
                    input_column ("odstmm",     "", overlays[idx].date_start >> 8, 2, 2);
                    input_column ("odstdd",     "", overlays[idx].date_start & 0xFF, 2, 2);
                }
            }
            else
            {
                text_column ("");
                text_column ("");
            }

            if (overlays[idx].date_code == OVERLAY_DATE_CODE_NONE && overlays[idx].date_start == 0)
            {
                text_column ("");
            }
            else
            {
                input_column ("od", "", overlays[idx].days, 2, 2);
            }

            save_column (id);
            begin_column ();
            http_send_FS ("<button type=\"submit\" name=\"action\" value=\"disp");
            http_send (disp);
            http_send_FS ("\">Display</button>");
            end_column ();
            end_table_row_form ();
        }

        table_trailer ();
    }

    if (n_overlays < MAX_OVERLAYS)
    {
        sprintf (id, "oid%d", n_overlays);

        begin_form (thispage);
        http_send_FS ("<table><tr>");
        text_column ("New overlay:");
        select_column ("otype", overlay_types, OVERLAY_TYPE_NONE, 0, N_OVERLAY_TYPES, 0);
        save_column (id);
        http_send_FS ("</tr></table>");
        end_form ();
    }

    if (message)
    {
        http_send_FS ("<P><font color=green>");
        http_send (message);
        http_send_FS ("</font>\r\n");
    }

    end_box ();
    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * ambilight page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_ambilight (void)
{
    const char *        thispage = "ambilight";
    const char *        header_cols[DISPLAY_HEADER_COLS] = { "Name", "Value", "Action" };
    const char *        ambimode_dec_cols[AMBILIGHT_MODE_DECELERATION_HEADER_COLS] = { "Name", "Deceleration", "Default", "Action" };
    static const char * ambilight_mode_names[MAX_AMBILIGHT_MODE_VARIABLES];
    static uint_fast8_t already_called;
    char *              action;
    const char *        message         = (const char *) 0;
    uint_fast8_t        use_rgbw        = get_numvar (DISPLAY_USE_RGBW_NUM_VAR);
    DSP_COLORS          rgbw;
    DSP_COLORS          rgbw_marker;
    const char *        ids[4]          = { "red", "green", "blue", "white" };
    const char *        marker_ids[4]   = { "mred", "mgreen", "mblue", "mwhite" };
    const char *        desc[4]         = { "R", "G", "B", "W" };
    char *              rgbw_buf[4];
    char *              rgbw_marker_buf[4];
    const char *        minval[4]       = { "0",   "0",  "0",  "0" };
    const char *        maxval[4]       = { "63", "63", "63", "63" };
    char                rbdecbuf[MAX_RAINBOW_DECELERATION_LEN + 1];
    AMBILIGHT_MODE *    am;
    uint_fast8_t        ambilight_brightness;
    uint_fast8_t        auto_brightness_active;
    char                brbuf[MAX_BRIGHTNESS_LEN + 1];
    char                red_buf[MAX_COLOR_VALUE_LEN + 16];
    char                green_buf[MAX_COLOR_VALUE_LEN + 16];
    char                blue_buf[MAX_COLOR_VALUE_LEN + 16];
    char                white_buf[MAX_COLOR_VALUE_LEN + 16];
    char                marker_red_buf[MAX_COLOR_VALUE_LEN + 16];
    char                marker_green_buf[MAX_COLOR_VALUE_LEN + 16];
    char                marker_blue_buf[MAX_COLOR_VALUE_LEN + 16];
    char                marker_white_buf[MAX_COLOR_VALUE_LEN + 16];
    char                ambimode_idbuf[8];
    char                ambimode_decidbuf[8];
    char                ambimode_defaultidbuf[8];
    char                ambimode_decbuf[MAX_AMBILIGHT_MODE_DECELERATION_LEN + 1];
    int                 ambilight_mode;
    int                 ambilight_leds;
    int                 ambilight_offset;
    int                 ambilight_markers = 0;
    uint_fast8_t        display_flags;
    uint_fast8_t        sync_ambilight;
    uint_fast8_t        sync_clock_markers;
    uint_fast8_t        fade_clock_seconds;
    uint_fast8_t        n_colors;
    uint_fast8_t        idx;
    uint_fast8_t        rtc             = 0;

    if (! already_called)
    {
        for (idx = 0; idx < MAX_AMBILIGHT_MODE_VARIABLES; idx++)
        {
            AMBILIGHT_MODE * am = get_ambilight_mode_var ((AMBILIGHT_MODE_VARIABLE) idx);
            ambilight_mode_names[idx] = am->name;
        }
        already_called = 1;
    }

    ambilight_mode          = get_numvar (AMBILIGHT_MODE_NUM_VAR);
    ambilight_leds          = get_numvar (AMBILIGHT_LEDS_NUM_VAR);
    ambilight_offset        = get_numvar (AMBILIGHT_OFFSET_NUM_VAR);
    display_flags           = get_numvar (DISPLAY_FLAGS_NUM_VAR);
    sync_ambilight          = (display_flags & DISPLAY_FLAGS_SYNC_AMBILIGHT) ? 1 : 0;
    sync_clock_markers      = (display_flags & DISPLAY_FLAGS_SYNC_CLOCK_MARKERS) ? 1 : 0;
    fade_clock_seconds      = (display_flags & DISPLAY_FLAGS_FADE_CLOCK_SECONDS) ? 1 : 0;
    ambilight_brightness    = get_numvar (AMBILIGHT_BRIGHTNESS_NUM_VAR);
    auto_brightness_active  = get_numvar (DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE_NUM_VAR);
    get_dsp_color_var (AMBILIGHT_DSP_COLOR_VAR, &rgbw);
    get_dsp_color_var (AMBILIGHT_MARKER_DSP_COLOR_VAR, &rgbw_marker);

    action = http_get_param ("action");

    if (*action)
    {
        if (! strcmp (action, "savesyncambi"))
        {
            if (http_get_checkbox_param ("syncambi"))
            {
                sync_ambilight = 1;
                display_flags |= DISPLAY_FLAGS_SYNC_AMBILIGHT;
            }
            else
            {
                sync_ambilight = 0;
                display_flags &= ~DISPLAY_FLAGS_SYNC_AMBILIGHT;
            }

            set_numvar (DISPLAY_FLAGS_NUM_VAR, display_flags);
        }
        else if (! strcmp (action, "savesyncmark"))
        {
            if (http_get_checkbox_param ("syncmark"))
            {
                sync_clock_markers = 1;
                display_flags |= DISPLAY_FLAGS_SYNC_CLOCK_MARKERS;
            }
            else
            {
                sync_clock_markers = 0;
                display_flags &= ~DISPLAY_FLAGS_SYNC_CLOCK_MARKERS;
            }

            set_numvar (DISPLAY_FLAGS_NUM_VAR, display_flags);
        }
        else if (! strcmp (action, "savefadeclk"))
        {
            if (http_get_checkbox_param ("fadeclk"))
            {
                fade_clock_seconds = 1;
                display_flags |= DISPLAY_FLAGS_FADE_CLOCK_SECONDS;
            }
            else
            {
                fade_clock_seconds = 0;
                display_flags &= ~DISPLAY_FLAGS_FADE_CLOCK_SECONDS;
            }

            set_numvar (DISPLAY_FLAGS_NUM_VAR, display_flags);
        }
        else if (! strcmp (action, "savebrightness"))
        {
            ambilight_brightness = atoi (http_get_param ("brightness"));
            set_numvar (AMBILIGHT_BRIGHTNESS_NUM_VAR, ambilight_brightness);
        }
        else if (! strcmp (action, "savecolors"))
        {
            rgbw.red     = atoi (http_get_param ("red"));
            rgbw.green   = atoi (http_get_param ("green"));
            rgbw.blue    = atoi (http_get_param ("blue"));

            if (use_rgbw)
            {
                rgbw.white = atoi (http_get_param ("white"));
            }
            else
            {
                rgbw.white = 0;
            }

            set_dsp_color_var (AMBILIGHT_DSP_COLOR_VAR, &rgbw, use_rgbw);
        }
        else if (! strcmp (action, "savemcolors"))
        {
            rgbw_marker.red     = atoi (http_get_param ("mred"));
            rgbw_marker.green   = atoi (http_get_param ("mgreen"));
            rgbw_marker.blue    = atoi (http_get_param ("mblue"));

            if (use_rgbw)
            {
                rgbw_marker.white = atoi (http_get_param ("mwhite"));
            }
            else
            {
                rgbw_marker.white = 0;
            }

            set_dsp_color_var (AMBILIGHT_MARKER_DSP_COLOR_VAR, &rgbw_marker, use_rgbw);
        }
        else if (! strcmp (action, "saveambimode"))
        {
            ambilight_mode = atoi (http_get_param ("ambimode"));
            set_numvar (AMBILIGHT_MODE_NUM_VAR, ambilight_mode);
        }
        else if (! strcmp (action, "saveambileds"))
        {
            ambilight_leds = atoi (http_get_param ("ambileds"));
            set_numvar (AMBILIGHT_LEDS_NUM_VAR, ambilight_leds);
        }
        else if (! strcmp (action, "saveambioffset"))
        {
            ambilight_offset = atoi (http_get_param ("ambioffset"));
            set_numvar (AMBILIGHT_OFFSET_NUM_VAR, ambilight_offset);
        }
        else if (! strncmp (action, "saveaan", 7))
        {
            uint_fast8_t    ambilight_mode_idx;
            uint_fast8_t    ambilight_mode_deceleration;

            ambilight_mode_idx = atoi (action + 7);

            if (ambilight_mode_idx < MAX_AMBILIGHT_MODE_VARIABLES)
            {
                sprintf (ambimode_decidbuf, "asp%d", ambilight_mode_idx);

                ambilight_mode_deceleration = atoi (http_get_param (ambimode_decidbuf));

                if (ambilight_mode_deceleration <= AMBILIGHT_MODE_MAX_DECELERATION)
                {
                    set_ambilight_mode_deceleration ((AMBILIGHT_MODE_VARIABLE) ambilight_mode_idx, ambilight_mode_deceleration);
                }
            }
        }
        else if (! strncmp (action, "adef", 4))
        {
            uint_fast8_t    ambilight_mode_idx;

            ambilight_mode_idx = atoi (action + 4);

            if (ambilight_mode_idx < MAX_AMBILIGHT_MODE_VARIABLES)
            {
                am = get_ambilight_mode_var ((AMBILIGHT_MODE_VARIABLE) ambilight_mode_idx);
                set_ambilight_mode_deceleration ((AMBILIGHT_MODE_VARIABLE) ambilight_mode_idx, am->default_deceleration);
            }
        }
        else if (! strcmp (action, "savemarkers"))
        {
            am = get_ambilight_mode_var (CLOCK_AMBILIGHT_MODE_VAR);
            int flags = am->flags;

            if (http_get_checkbox_param ("markers"))
            {
                flags |= AMBILIGHT_FLAG_SECONDS_MARKER;
            }
            else
            {
                flags &= ~AMBILIGHT_FLAG_SECONDS_MARKER;
            }

            set_ambilight_mode_flags (CLOCK_AMBILIGHT_MODE_VAR, flags);
        }
    }

    am = get_ambilight_mode_var (CLOCK_AMBILIGHT_MODE_VAR);

    if (am->flags & AMBILIGHT_FLAG_SECONDS_MARKER)
    {
        ambilight_markers = 1;
    }

    am = get_ambilight_mode_var (RAINBOW_AMBILIGHT_MODE_VAR);

    sprintf (rbdecbuf,              "%d", am->deceleration);
    sprintf (brbuf,                 "%d", ambilight_brightness);

    sprintf (red_buf,               "%d", rgbw.red);
    sprintf (green_buf,             "%d", rgbw.green);
    sprintf (blue_buf,              "%d", rgbw.blue);

    sprintf (marker_red_buf,        "%d", rgbw_marker.red);
    sprintf (marker_green_buf,      "%d", rgbw_marker.green);
    sprintf (marker_blue_buf,       "%d", rgbw_marker.blue);

    if (use_rgbw)
    {
        sprintf (white_buf, "%d", rgbw.white);
        sprintf (marker_white_buf, "%d", rgbw_marker.white);
    }
    else
    {
        white_buf[0] = '0';
        white_buf[1] = '\0';
        marker_white_buf[0] = '0';
        marker_white_buf[1] = '\0';
    }

    rgbw_buf[0] = red_buf;
    rgbw_buf[1] = green_buf;
    rgbw_buf[2] = blue_buf;
    rgbw_buf[3] = white_buf;

    rgbw_marker_buf[0] = marker_red_buf;
    rgbw_marker_buf[1] = marker_green_buf;
    rgbw_marker_buf[2] = marker_blue_buf;
    rgbw_marker_buf[3] = marker_white_buf;

    http_header ("Ambilight", (const char *) NULL, (const char *) NULL);
    begin_box ("Ambilight");

    table_header (header_cols, DISPLAY_HEADER_COLS);

    table_row_input (thispage, 3, "#LEDs", "ambileds", ambilight_leds, 3);
    table_row_input (thispage, 3, "Offset of second = 0", "ambioffset", ambilight_offset, 3);

    if (use_rgbw)
    {
        n_colors = 4;
    }
    else
    {
        n_colors = 3;
    }

    table_row_checkbox (thispage, "Ambilight", "syncambi", "Use display colors", sync_ambilight);

    if (! sync_ambilight)
    {
        if (! auto_brightness_active)
        {
            table_row_slider (thispage, "Brightness (1-15)", "brightness", brbuf, "0", "15");
        }

        table_row_sliders (thispage, "Colors", "colors", n_colors, ids, desc, rgbw_buf, minval, maxval);
    }

    table_row_checkbox (thispage, "Marker Colors", "syncmark", "Use display colors", sync_clock_markers);

    if (! sync_clock_markers)
    {
        table_row_sliders (thispage, "Marker Colors", "mcolors", n_colors, marker_ids, desc, rgbw_marker_buf, minval, maxval);
    }

    table_row_select (thispage, "Ambilight Mode", "ambimode", ambilight_mode_names, ambilight_mode, 0, MAX_AMBILIGHT_MODE_VARIABLES, 0);
    table_trailer ();

    table_header (ambimode_dec_cols, AMBILIGHT_MODE_DECELERATION_HEADER_COLS);

    for (idx = 0; idx < MAX_AMBILIGHT_MODE_VARIABLES; idx++)
    {
        AMBILIGHT_MODE * am = get_ambilight_mode_var ((AMBILIGHT_MODE_VARIABLE) idx);

        if (am->flags & AMBILIGHT_FLAG_CONFIGURABLE)
        {
            sprintf (ambimode_idbuf, "aan%d", idx);
            sprintf (ambimode_decidbuf, "asp%d", idx);
            sprintf (ambimode_defaultidbuf, "adef%d", idx);
            sprintf (ambimode_decbuf, "%d", am->deceleration);

            begin_table_row_form (thispage);
            text_column (am->name);
            slider_column (ambimode_decidbuf, ambimode_decbuf, "0", "15");
            button_column (ambimode_defaultidbuf, "Default");
            save_column (ambimode_idbuf);
            end_table_row_form ();
        }
    }

    table_row_checkbox (thispage, "Clock", "markers", "Enable 5-second markers", ambilight_markers);
    table_row_checkbox (thispage, "Clock", "fadeclk", "Fade clock seconds", fade_clock_seconds);

    table_trailer ();
    begin_form (thispage);
    end_form ();

    if (message)
    {
        http_send_FS ("<P><font color=green>");
        http_send (message);
        http_send_FS ("</font>\r\n");
    }

    end_box ();
    http_trailer ();
    http_flush ();

    return rtc;
}

static void
table_row_timers (const char * page, uint_fast8_t is_ambilight, int idx)
{
    char            id[8];
    char            idx_buf[3];
    char            hour_buf[16];
    char            minute_buf[16];
    char            act_id[8];
    char            on_id[8];
    char            hour_id[8];
    char            min_id[8];
    char            day_id[8];
    NIGHT_TIME *    nt;

    nt = get_night_time_var (is_ambilight, (NIGHT_TIME_VARIABLE) idx);

    if (nt)
    {
        sprintf (hour_buf,   "%02d", nt->minutes / 60);
        sprintf (minute_buf, "%02d", nt->minutes % 60);

        begin_table_row_form (page);

        sprintf (idx_buf, "%d",   idx);
        sprintf (id,      "id%d", idx);
        sprintf (act_id,  "a%d",  idx);
        sprintf (on_id,   "o%d",  idx);
        sprintf (hour_id, "h%d",  idx);
        sprintf (min_id,  "m%d",  idx);

        text_column (idx_buf);

        checkbox_column (act_id, "", (nt->flags & NIGHT_TIME_FLAG_ACTIVE) ? 1 : 0);
        checkbox_column (on_id, "", (nt->flags & NIGHT_TIME_FLAG_SWITCH_ON) ? 1 : 0);

        sprintf (day_id, "f%d", idx);
        select_column (day_id, wdays_en, (nt->flags & NIGHT_TIME_FROM_DAY_MASK) >> 3, 0, 7, 0);

        sprintf (day_id, "t%d", idx);
        select_column (day_id, wdays_en, (nt->flags & NIGHT_TIME_TO_DAY_MASK), 0, 7, 0);

        input_column (hour_id, "",  hour_buf, 2, 2);
        input_column (min_id, "",  minute_buf, 2, 2);

        save_column (id);
        end_table_row_form ();
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * timers page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_timers (uint_fast8_t is_ambilight)
{
    const char *        header_cols[TIMERS_HEADER_COLS] = { "Slot", "Active", "On", "From", "To", "Hour", "Min", "Action" };
    const char *        thispage;
    char *              action;
    const char *        title;
    uint_fast8_t        idx;
    uint_fast8_t        rtc = 0;

    action = http_get_param ("action");

    if (*action)
    {
        if (! strncmp (action, "saveid", 6))
        {
            char id[16];
            uint_fast8_t idx = atoi (action + 6);

            if (idx < MAX_NIGHT_TIME_VARIABLES)
            {
                NIGHT_TIME *    nt;
                uint_fast8_t    from_day;
                uint_fast8_t    to_day;
                uint_fast16_t   minutes;
                uint_fast8_t    flags;

                nt = get_night_time_var (is_ambilight, (NIGHT_TIME_VARIABLE) idx);

                flags = nt->flags;

                if (http_get_checkbox_param_by_idx ("a", idx))
                {
                    flags |= NIGHT_TIME_FLAG_ACTIVE;
                }
                else
                {
                    flags &= ~NIGHT_TIME_FLAG_ACTIVE;
                }

                if (http_get_checkbox_param_by_idx ("o", idx))
                {
                    flags |= NIGHT_TIME_FLAG_SWITCH_ON;
                }
                else
                {
                    flags &= ~NIGHT_TIME_FLAG_SWITCH_ON;
                }

                sprintf (id, "f%d", idx);
                from_day = atoi (http_get_param (id));
                sprintf (id, "t%d", idx);
                to_day = atoi (http_get_param (id));

                flags &= ~(NIGHT_TIME_FROM_DAY_MASK | NIGHT_TIME_TO_DAY_MASK);
                flags |= NIGHT_TIME_FROM_DAY_MASK & (from_day << 3);
                flags |= NIGHT_TIME_TO_DAY_MASK & (to_day);

                minutes = atoi (http_get_param_by_idx ("h", idx)) * 60 + atoi (http_get_param_by_idx ("m", idx));

                set_night_time_var (is_ambilight, (NIGHT_TIME_VARIABLE) idx, minutes, flags);
            }
        }
    }

    if (is_ambilight)
    {
        title = "Ambilight Timers";
        thispage = "atimers";
    }
    else
    {
        title = "Timers";
        thispage = "timers";
    }

    http_header (title, (const char *) NULL, (const char *) NULL);
    begin_box (title);

    table_header (header_cols, TIMERS_HEADER_COLS);

    for (idx = 0; idx < MAX_NIGHT_TIME_VARIABLES; idx++)
    {
        table_row_timers (thispage, is_ambilight, idx);
    }

    table_trailer ();

    end_box ();
    http_trailer ();
    http_flush ();

    return rtc;
}

static void
table_row_alarm_timers (const char * page, int idx)
{
    char            id[8];
    char            idx_buf[8];
    char            hour_buf[16];
    char            minute_buf[16];
    char            act_id[8];
    char            on_id[8];
    char            hour_id[8];
    char            min_id[8];
    char            day_id[8];
    ALARM_TIME *    at;

    at = get_alarm_time_var ((ALARM_TIME_VARIABLE) idx);

    if (at)
    {
        sprintf (hour_buf,   "%02d", at->minutes / 60);
        sprintf (minute_buf, "%02d", at->minutes % 60);

        begin_table_row_form (page);

        sprintf (idx_buf, "%03d.mp3", idx + 1);                             // 001.mp3 ... 008.mp3
        sprintf (id,      "id%d",     idx);
        sprintf (act_id,  "a%d",      idx);
        sprintf (on_id,   "o%d",      idx);
        sprintf (hour_id, "h%d",      idx);
        sprintf (min_id,  "m%d",      idx);

        text_column (idx_buf);

        checkbox_column (act_id, "", (at->flags & ALARM_TIME_FLAG_ACTIVE) ? 1 : 0);

        sprintf (day_id, "f%d", idx);
        select_column (day_id, wdays_en, (at->flags & ALARM_TIME_FROM_DAY_MASK) >> 3, 0, 7, 0);

        sprintf (day_id, "t%d", idx);
        select_column (day_id, wdays_en, (at->flags & ALARM_TIME_TO_DAY_MASK), 0, 7, 0);

        input_column (hour_id, "",  hour_buf, 2, 2);
        input_column (min_id, "",  minute_buf, 2, 2);

        save_column (id);
        end_table_row_form ();
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * dfplayer page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define PLAY_TRACK_HEADER_COLS  3
#define DFPLAYER_MAX_VOLUME     30
#define MAX_SILENCE_START_LEN   4
#define MAX_SILENCE_STOP_LEN    4

static uint_fast8_t
http_dfplayer (void)
{
    const char *    thispage = "dfplayer";
    const char *    dfplayer_header_cols[DFPLAYER_HEADER_COLS]      = { "Name", "Value", "Action" };
    const char *    dfplayer_silence_cols[DFPLAYER_SILENCE_COLS]    =  { "Name", "Hour", "Min", "Action" };
    const char *    alarm_header_cols[ALARM_TIMERS_HEADER_COLS]     = { "Track", "Active", "From", "To", "Hour", "Min", "Action" };
    const char *    play_track_header_cols[PLAY_TRACK_HEADER_COLS]  = { "Folder", "Track", "Action" };
    const char *    dfplayer_mode_names[3]                          = { "None", "Bell", "Speak" };
    uint_fast8_t    max_dfplayer_modes = 3;
    char            txtbuf[32];
    char            hour_buf[16];
    char            minute_buf[16];
    char            valbuf[32];
    char            maxbuf[32];
    uint_fast8_t    idx;
    uint_fast8_t    volume;
    uint_fast16_t   silence_start;
    uint_fast16_t   silence_stop;
    uint_fast8_t    dfplayer_mode;
    uint_fast8_t    bell_flags;
    uint_fast8_t    speak_cycle;
    uint_fast8_t    folder;
    uint_fast8_t    track;
    char *          action;
    uint_fast8_t    rtc = 0;

    action = http_get_param ("action");

    if (*action)
    {
        if (! strcmp (action, "savevolume"))
        {
            volume = atoi (http_get_param (action + 4));

            if (volume > DFPLAYER_MAX_VOLUME)
            {
                volume = DFPLAYER_MAX_VOLUME;
            }

            set_numvar (DFPLAYER_VOLUME_NUM_VAR, volume);
        }
        else if (! strcmp (action, "savesilstart"))
        {
            uint_fast16_t   silstarth;
            uint_fast16_t   silstartm;

            silstarth = atoi (http_get_param ("silstarth"));
            silstartm = atoi (http_get_param ("silstartm"));

            if (silstarth < 24 && silstartm < 60)
            {
                silence_start = 60 * silstarth + silstartm;
                set_numvar (DFPLAYER_SILENCE_START_NUM_VAR, silence_start);
            }
        }
        else if (! strcmp (action, "savesilstop"))
        {
            uint_fast16_t   silstoph;
            uint_fast16_t   silstopm;

            silstoph = atoi (http_get_param ("silstoph"));
            silstopm = atoi (http_get_param ("silstopm"));

            if (silstoph < 24 && silstopm < 60)
            {
                silence_stop = 60 * silstoph + silstopm;
                set_numvar (DFPLAYER_SILENCE_STOP_NUM_VAR, silence_stop);
            }
        }
        else if (! strcmp (action, "savemode"))
        {
            dfplayer_mode = atoi (http_get_param (action + 4));
            set_numvar (DFPLAYER_MODE_NUM_VAR, dfplayer_mode);
        }
        else if (! strcmp (action, "savebell"))
        {
            bell_flags = DFPLAYER_MODE_BELL_FLAG_NONE;

            if (http_get_checkbox_param ("m15"))
            {
                bell_flags |= DFPLAYER_MODE_BELL_FLAG_15;
            }
            if (http_get_checkbox_param ("m30"))
            {
                bell_flags |= DFPLAYER_MODE_BELL_FLAG_30;
            }
            if (http_get_checkbox_param ("m45"))
            {
                bell_flags |= DFPLAYER_MODE_BELL_FLAG_45;
            }

            set_numvar (DFPLAYER_BELL_FLAGS_NUM_VAR, bell_flags);
        }
        else if (! strcmp (action, "savespeak"))
        {
            speak_cycle = atoi (http_get_param (action + 4));
            set_numvar (DFPLAYER_SPEAK_CYCLE_NUM_VAR, speak_cycle);
        }
        else if (! strncmp (action, "saveid", 6))
        {
            char id[16];

            idx = atoi (action + 6);

            if (idx < MAX_ALARM_TIME_VARIABLES)
            {
                ALARM_TIME *    at;
                uint_fast8_t    from_day;
                uint_fast8_t    to_day;
                uint_fast16_t   minutes;
                uint_fast8_t    flags;

                at = get_alarm_time_var ((ALARM_TIME_VARIABLE) idx);

                flags = at->flags;

                if (http_get_checkbox_param_by_idx ("a", idx))
                {
                    flags |= ALARM_TIME_FLAG_ACTIVE;
                }
                else
                {
                    flags &= ~ALARM_TIME_FLAG_ACTIVE;
                }

                sprintf (id, "f%d", idx);
                from_day = atoi (http_get_param (id));
                sprintf (id, "t%d", idx);
                to_day = atoi (http_get_param (id));

                flags &= ~(ALARM_TIME_FROM_DAY_MASK | ALARM_TIME_TO_DAY_MASK);
                flags |= ALARM_TIME_FROM_DAY_MASK & (from_day << 3);
                flags |= ALARM_TIME_TO_DAY_MASK & (to_day);

                minutes = atoi (http_get_param_by_idx ("h", idx)) * 60 + atoi (http_get_param_by_idx ("m", idx));

                set_alarm_time_var ((ALARM_TIME_VARIABLE) idx, minutes, flags);
            }
        }
        else if (! strcmp (action, "play"))
        {
            folder = atoi (http_get_param ("plfolder"));
            track  = atoi (http_get_param ("pltrack"));
            set_numvar (DFPLAYER_PLAY_FOLDER_TRACK_NUM_VAR, folder << 8 | track);
        }
    }

    http_header ("DFPlayer", (const char *) NULL, (const char *) NULL);
    begin_box ("DFPlayer");

    volume        = get_numvar (DFPLAYER_VOLUME_NUM_VAR);
    silence_start = get_numvar (DFPLAYER_SILENCE_START_NUM_VAR);
    silence_stop  = get_numvar (DFPLAYER_SILENCE_STOP_NUM_VAR);
    dfplayer_mode = get_numvar (DFPLAYER_MODE_NUM_VAR);
    bell_flags    = get_numvar (DFPLAYER_BELL_FLAGS_NUM_VAR);
    speak_cycle   = get_numvar (DFPLAYER_SPEAK_CYCLE_NUM_VAR);

    table_header (dfplayer_header_cols, DFPLAYER_HEADER_COLS);

    sprintf (txtbuf, "Volume (0-%d)", DFPLAYER_MAX_VOLUME);
    sprintf (valbuf, "%d", volume);
    sprintf (maxbuf, "%d", DFPLAYER_MAX_VOLUME);
    table_row_slider (thispage, txtbuf, "volume", valbuf, "0", maxbuf);
    table_row_select (thispage, "Mode", "mode", dfplayer_mode_names, dfplayer_mode, 0, max_dfplayer_modes, 0);

    switch (dfplayer_mode)
    {
        case DFPLAYER_MODE_BELL:
        {
            begin_table_row_form (thispage);
            text_column ("Bell");
            http_send_FS ("<td>");
            checkbox_field ("m15", "xx:15", (bell_flags & DFPLAYER_MODE_BELL_FLAG_15) ? 1 : 0);
            checkbox_field ("m30", "xx:30", (bell_flags & DFPLAYER_MODE_BELL_FLAG_30) ? 1 : 0);
            checkbox_field ("m45", "xx:45", (bell_flags & DFPLAYER_MODE_BELL_FLAG_45) ? 1 : 0);
            http_send_FS ("</td>");
            save_column ("bell");
            end_table_row_form ();
            break;
        }
        case DFPLAYER_MODE_SPEAK:
        {
            table_row_input (thispage, 3, "Speak cycle", "speak", speak_cycle, 3);
            break;
        }
    }

    table_trailer ();

    table_header (dfplayer_silence_cols, DFPLAYER_SILENCE_COLS);

    begin_table_row_form (thispage);
    text_column ("Silence start");
    sprintf (hour_buf,   "%02d", silence_start / 60);
    sprintf (minute_buf, "%02d", silence_start % 60);
    input_column ("silstarth", "",  hour_buf, 2, 2);
    input_column ("silstartm", "",  minute_buf, 2, 2);
    save_column ("silstart");
    end_table_row_form ();

    begin_table_row_form (thispage);
    text_column ("Silence stop");
    sprintf (hour_buf,   "%02d", silence_stop / 60);
    sprintf (minute_buf, "%02d", silence_stop % 60);
    input_column ("silstoph", "",  hour_buf, 2, 2);
    input_column ("silstopm", "",  minute_buf, 2, 2);
    save_column ("silstop");
    end_table_row_form ();

    table_trailer ();

    table_header (alarm_header_cols, ALARM_TIMERS_HEADER_COLS);

    for (idx = 0; idx < MAX_ALARM_TIME_VARIABLES; idx++)
    {
        table_row_alarm_timers (thispage, idx);
    }

    table_trailer ();

    table_header (play_track_header_cols, PLAY_TRACK_HEADER_COLS);

    begin_table_row_form (thispage, 0);
    input_column ("plfolder", "",  "", 2, 2);
    input_column ("pltrack",  "",  "", 3, 3);
    button_column ("play", "Play");
    end_table_row_form ();

    table_trailer ();

    end_box ();
    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * tft page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define SSD1963_GLOBAL_FLAGS_RGB_ORDER          0x01
#define SSD1963_GLOBAL_FLAGS_FLIP_HORIZONTAL    0x02
#define SSD1963_GLOBAL_FLAGS_FLIP_VERTICAL      0x04
#define SSD1963_GLOBAL_FLAGS_MASK               0x07

#define TFT_HEADER_COLS                         3

static uint_fast8_t
http_tft (void)
{
    const char *    thispage = "tft";
    const char *    tft_header_cols[TFT_HEADER_COLS]           = { "Name", "Value", "Action" };
    uint_fast8_t    flags;
    char *          action;
    uint_fast8_t    rtc = 0;

    flags = get_numvar (SSD1963_FLAGS_NUM_VAR);
    action = http_get_param ("action");

    if (*action)
    {
        if (! strcmp (action, "saveflags"))
        {
            flags = 0;

            if (http_get_checkbox_param ("rgb"))
            {
                flags |= SSD1963_GLOBAL_FLAGS_RGB_ORDER;
            }
            
            if (http_get_checkbox_param ("hflip"))
            {
                flags |= SSD1963_GLOBAL_FLAGS_FLIP_HORIZONTAL;
            }

            if (http_get_checkbox_param ("vflip"))
            {
                flags |= SSD1963_GLOBAL_FLAGS_FLIP_VERTICAL;
            }

            set_numvar (SSD1963_FLAGS_NUM_VAR, flags);
        }
    }

    http_header ("TFT", (const char *) NULL, (const char *) NULL);
    begin_box ("TFT");

    table_header (tft_header_cols, TFT_HEADER_COLS);

    begin_table_row_form (thispage);
    text_column ("Color<BR>Hor.<BR>Vert.");
    http_send_FS ("<td align=right>");
    checkbox_field ("rgb", "RGB", (flags & SSD1963_GLOBAL_FLAGS_RGB_ORDER) ? 1 : 0);
    http_send_FS ("<br>");
    checkbox_field ("hflip", "Flip", (flags & SSD1963_GLOBAL_FLAGS_FLIP_HORIZONTAL) ? 1 : 0);
    http_send_FS ("<br>");
    checkbox_field ("vflip", "Flip", (flags & SSD1963_GLOBAL_FLAGS_FLIP_VERTICAL) ? 1 : 0);
    http_send_FS ("</td>");
    save_column ("flags");
    end_table_row_form ();

    table_trailer ();

    end_box ();
    http_trailer ();
    http_flush ();

    return rtc;
}

static bool
download_file_to_local (const char * host, const char * path, const char * remote_filename, const char * local_filename)
{
    int             len;                                                // content len
    bool            rtc = false;

    len = httpclient (host, path, remote_filename);

    if (len > 0)
    {
        int ch;
        int idx = 0;

        File f = LittleFS.open(local_filename, "w");

        if (! f)
        {
            httpclient_stop ();
            return false;
        }

        bool write_ok = true;

        while (len > 0)
        {
            ch = httpclient_read (&len);

            if (ch < 0)
            {
                break;
            }

            http_download_buf[idx++] = ch;
            if (idx == 1024)
            {
               /* File::write () liefert die tatsaechlich geschriebene Menge und faellt
                * bei vollem LittleFS kleiner aus. Ungeprueft bliebe eine verkuerzte
                * Datei liegen, die jede Existenz- und Groessenpruefung besteht - bei
                * einem .gz-Asset ist genau das der dokumentierte Weissschirm.
                */
               if (f.write (http_download_buf, (size_t) idx) != (size_t) idx)
               {
                   write_ok = false;
                   break;
               }

               idx = 0;
            }
        }

        if (write_ok && idx > 0 && f.write (http_download_buf, (size_t) idx) != (size_t) idx)
        {
            write_ok = false;
        }

        f.close ();
        httpclient_stop ();

        if (! write_ok)
        {
            Serial.print ("- fs write short: ");
            Serial.println (local_filename);
            Serial.flush ();
            LittleFS.remove (local_filename);               // lieber keine Datei als eine halbe
        }

        rtc = (write_ok && len == 0);
    }
    return rtc;
}

static bool
download_file (const char * host, const char * path, const char * filename)
{
    return download_file_to_local (host, path, filename, filename);
}

#define READ_LINE_TIMEOUT   2000  // 2000 msec
#define READ_BODY_TIMEOUT   5000  // 5000 msec

static bool
read_line (String& line)
{
    int c = 0;
    ulong start_millis = 0;
    bool  rtc = true;

    line = "";

    start_millis = millis();

    do
    {
        if (http_client.available())
        {
            start_millis = millis();
            c = http_client.read();
    
            if (c >= 0 && c != '\n' && c != '\r')
            {
                line += (char) c;
            }
        }
        else
        {
            if ((millis() - start_millis) >= READ_LINE_TIMEOUT)
            {
                rtc = false;
                break;
            }
        }
    } while (c >= 0 && c != '\r');

    yield();
    return (rtc);
}

static bool
read_post_headers (size_t * content_length)
{
    String line;

    *content_length = 0;
    http_clear_request_user_agent ();

    while (read_line (line))
    {
        if (line == "")
        {
            if (http_client.available () && http_client.peek () == '\n')
            {
                http_client.read ();
            }
            return true;
        }
        else if (line.startsWith ("Content-Length:"))
        {
            *content_length = (size_t) strtoul (line.substring (15).c_str (), NULL, 10);
        }
        else if (line.startsWith ("User-Agent:"))
        {
            http_capture_request_user_agent (line);
        }
    }

    return false;
}

static bool
read_request_body_to_file (File& f, size_t content_length)
{
    unsigned long start_millis = millis ();

    while (content_length > 0)
    {
        if (http_client.available ())
        {
            uint8_t buf[256];
            size_t  chunk = http_client.available ();

            if (chunk > sizeof (buf))
            {
                chunk = sizeof (buf);
            }

            if (chunk > content_length)
            {
                chunk = content_length;
            }

            int n = http_client.read (buf, chunk);

            if (n > 0)
            {
                /* Kurzschreibung des Dateisystems: bei vollem LittleFS nimmt
                 * File::write () weniger an als uebergeben. Ungeprueft landete eine
                 * verkuerzte Datei auf dem Geraet und der Aufrufer meldete Erfolg -
                 * bei app.js.gz ist das der dokumentierte Weissschirm. Der Aufrufer
                 * entfernt die Datei, wenn hier false zurueckkommt.
                 */
                if (f.write (buf, (size_t) n) != (size_t) n)
                {
                    Serial.println ("- fs write short on upload");
                    Serial.flush ();
                    return false;
                }

                content_length -= n;
                start_millis = millis ();
                yield ();
            }
        }
        else if ((millis () - start_millis) >= READ_BODY_TIMEOUT)
        {
            return false;
        }
    }

    return true;
}

static size_t
read_request_body_to_update (size_t content_length)
{
    size_t written = 0;
    unsigned long start_millis = millis ();

    while (written < content_length)
    {
        if (http_client.available ())
        {
            uint8_t buf[256];
            size_t  chunk = http_client.available ();

            if (chunk > sizeof (buf))
            {
                chunk = sizeof (buf);
            }

            if (chunk > content_length - written)
            {
                chunk = content_length - written;
            }

            int n = http_client.read (buf, chunk);

            if (n > 0)
            {
                size_t just_written = Update.write (buf, n);

                if (just_written != (size_t) n)
                {
                    return written + just_written;
                }

                written += just_written;
                start_millis = millis ();
                yield ();
            }
        }
        else if ((millis () - start_millis) >= READ_BODY_TIMEOUT)
        {
            return written;
        }
    }

    return written;
}

static size_t
skip_leading_body_newlines (void)
{
    size_t skipped = 0;
    unsigned long start_millis = millis ();

    while ((millis () - start_millis) < READ_BODY_TIMEOUT)
    {
        if (http_client.available ())
        {
            int ch = http_client.peek ();

            if (ch == '\r' || ch == '\n')
            {
                http_client.read ();
                skipped++;
                continue;
            }
            break;
        }
    }

    return skipped;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * LittleFS page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define FSINFO_HEADER_COLS      2
#define DIRECTORY_HEADER_COLS   2

#define POST_ICON_NONE            0
#define POST_ICON_FILE            1
#define POST_ICON_WEATHER_FILE    2
#define POST_TABLES_FILE          3
#define POST_DISPLAY_FILE         4

static uint_fast8_t
http_fs (int post = POST_ICON_NONE)
{
    const char *    thispage = "fs";
    const char *    update_header_cols[UPDATE_HEADER_COLS]  = { "Name", "Value" };
    const char *    fsinfo_cols[FSINFO_HEADER_COLS]         = { "Parameter", "Value" };
    const char *    directory_cols[DIRECTORY_HEADER_COLS]   = { "File", "Size" };
    char *          action;
    char            valbuf[16];
    const char *    fname_icon      = (const char *) 0;
    const char *    fname_weather   = (const char *) 0;
    const char *    fname_tables    = (const char *) 0;
    const char *    fname_display   = (const char *) 0;
    const char *    show_fname      = (const char *) 0;
    const char *    tables_filter   = (const char *) 0;
    char *          update_host;
    char *          update_path;
    FSInfo          fsinfo;
    int             download_rtc = 2;
    STR_VAR *       sv;
    int             len;
    uint_fast8_t    rtc = 0;

    action = http_get_param ("action");

    if (hardware_configuration != 0xFFFF)
    {
        switch (hardware_configuration & HW_WC_MASK)
        {
            case HW_WC_24H:
                fname_icon      = "wc24h-icon.txt";
                fname_weather   = "wc24h-weather.txt";
                fname_tables    = "wc24h-tables-local.txt";
                fname_display   = "wc24h-display-local.txt";
                tables_filter   = "wc24h-tables-";
                break;
            case HW_WC_12H:
                fname_icon      = "wc12h-icon.txt";
                fname_weather   = "wc12h-weather.txt";
                fname_tables    = "wc12h-tables-local.txt";
                fname_display   = "wc12h-display-local.txt";
                tables_filter   = "wc12h-tables-";
                break;
            case HW_UCLOCK:
                fname_icon      = "uc-icon.txt";
                fname_weather   = "uc-weather.txt";
                fname_tables    = (const char *) 0;
                fname_display   = (const char *) 0;
                tables_filter   = (const char *) 0;
                break;
        }
    }

    sv = get_strvar (UPDATE_HOST_VAR);
    update_host = sv->str;

    if (! update_host[0])
    {
        update_host = (char *) DEFAULT_UPDATE_HOST;
    }

    sv = get_strvar (UPDATE_PATH_VAR);
    update_path = sv->str;

    if (! update_path[0])
    {
        update_path = (char *) DEFAULT_UPDATE_PATH;
    }

    if (post != POST_ICON_NONE)
    {
        File f = (File) 0;
        String line;
        bool   write_ok = true;
        LittleFS.begin();

        if (post == POST_ICON_FILE)
        {
            if (fname_icon)
            {
                f = LittleFS.open(fname_icon, "w+");
            }
        }
        else if (post == POST_ICON_WEATHER_FILE)
        {
            if (fname_weather)
            {
                f = LittleFS.open(fname_weather, "w+");
            }
        }
        else if (post == POST_TABLES_FILE)
        {
            if (fname_tables)
            {
                f = LittleFS.open(fname_tables, "w+");
            }
        }
        else if (post == POST_DISPLAY_FILE)
        {
            if (fname_display)
            {
                f = LittleFS.open(fname_display, "w+");
            }
        }
        if (f)
        {
            // find the line with content type and boundary string
            String boundary;

            while (read_line(line))
            {
                if (line.startsWith("Content-Type:"))
                {
                    int n = line.indexOf("boundary=");
                    if (n > 0)
                    {
                        boundary = "--" + line.substring(n + strlen("boundary="));
                    }
                    break;
                }
            }

            // no boundary? something is wrong
            if (boundary == "")
            {
                http_send("<font color='red'>Wrong http header</font><br/>");
            }
            else
            {
                // read until a boundary has been found
                while (read_line(line) && line != boundary)
                {
                    ;
                }

                // read the empty line
                while (read_line(line) && line != "")
                {
                    ;
                }

                // read and save file content until the boundary has been found
                while (read_line(line))
                {
                    // boundary found, end of file
                    if (line == boundary)
                    {
                        break;
                    }
                    /* File::write () nimmt bei vollem LittleFS weniger an als
                     * uebergeben. Ungeprueft lag danach eine verkuerzte Tabellen-,
                     * Icon- oder Wetterdatei auf dem Geraet, und die Seite meldete
                     * trotzdem Erfolg.
                     */
                    if (f.write((const unsigned char*)line.c_str(), line.length()) != line.length() ||
                        f.write('\r') != 1 || f.write('\n') != 1)
                    {
                        write_ok = false;
                        break;
                    }

                    yield ();
                }
            }

            f.close();

            if (! write_ok)
            {
                Serial.println ("- fs write short on legacy upload");
                Serial.flush ();
                http_send_FS ("<font color=red>Dateisystem voll &mdash; Datei unvollstaendig geschrieben.</font></B><BR>\r\n");
            }
        }
        else
        {
            http_send_FS ("<font color=red>Failed to upload file.</font></B><BR>\r\n");
        }

        LittleFS.end();
    }

    if (*action)
    {
        if (! strcmp (action, "format"))
        {
            LittleFS.begin ();
            LittleFS.format ();
            LittleFS.end ();
        }
        else if (! strcmp (action, "saveuphost"))
        {
            set_strvar (UPDATE_HOST_VAR, http_get_param ("uphost"));
        }
        else if (! strcmp (action, "saveuppath"))
        {
            set_strvar (UPDATE_PATH_VAR, http_get_param ("uppath"));
        }
        else if (! strcmp (action, "savedwnfil"))
        {
            char * fname = http_get_param ("dwnfil");

            LittleFS.begin ();
            download_rtc = download_file (update_host, update_path, fname);
            LittleFS.end ();
        }
        else if (! strcmp (action, "dwntable"))
        {
            char * fname = http_get_param ("tablefile");

            LittleFS.begin ();
            download_rtc = download_file (update_host, update_path, fname);
            LittleFS.end ();
            tables_init ();                                                         // reload layout tables
        }
        else if (! strcmp (action, "download"))
        {
            LittleFS.begin ();

            if (fname_icon)
            {
                download_rtc = download_file (update_host, update_path, fname_icon);
            }

            if (fname_weather)
            {
                download_rtc = download_file (update_host, update_path, fname_weather);
            }

            LittleFS.end ();
        }
        else if (! strcmp (action, "remove"))
        {
            char * fname = http_get_param ("filename");
            LittleFS.begin ();
            LittleFS.remove (fname);
            LittleFS.end ();
        }
        else if (! strcmp (action, "show"))
        {
            show_fname = http_get_param ("filename");
        }
    }

    http_header ("ESP8266 LittleFS", (const char *) NULL, (const char *) NULL);
    begin_box ("ESP8266 LittleFS");
    message_tables_file_missing ();

    if (tables_corrupt)
    {
        http_send_FS ("<font color=\"red\"><B>table file wcxx-tables-xx.txt is corrupt. Remove it, load it again or format LittleFS!</B></font><P>\r\n");
    }

    if (download_rtc == 0)
    {
        http_send_FS ("download failed<BR>");
    }
    else if (download_rtc == 1)
    {
        http_send_FS ("download successful<BR>");
    }

    sv = get_strvar (UPDATE_HOST_VAR);
    update_host = sv->str;
    sv = get_strvar (UPDATE_PATH_VAR);
    LittleFS.begin();

    table_header (fsinfo_cols, FSINFO_HEADER_COLS);

    if (LittleFS.info(fsinfo))
    {
        begin_table_row ();
        text_column ("Total bytes");
        sprintf (valbuf, "%d", fsinfo.totalBytes);
        text_rcolumn (valbuf);
        end_table_row ();

        begin_table_row ();
        text_column ("Used bytes");
        sprintf (valbuf, "%d", fsinfo.usedBytes);
        text_rcolumn (valbuf);
        end_table_row ();

        begin_table_row ();
        text_column ("Block size");
        sprintf (valbuf, "%d", fsinfo.blockSize);
        text_rcolumn (valbuf);
        end_table_row ();

        begin_table_row ();
        text_column ("Page size");
        sprintf (valbuf, "%d", fsinfo.pageSize);
        text_rcolumn (valbuf);
        end_table_row ();

        begin_table_row ();
        text_column ("Max open files");
        sprintf (valbuf, "%d", fsinfo.maxOpenFiles);
        text_rcolumn (valbuf);
        end_table_row ();

        begin_table_row ();
        text_column ("Max path length");
        sprintf (valbuf, "%d", fsinfo.maxPathLength);
        text_rcolumn (valbuf);
        end_table_row ();
    }

    table_trailer ();

    Dir dir = LittleFS.openDir("");
  
    table_header (directory_cols, DIRECTORY_HEADER_COLS);
  
    while (dir.next())
    {
        begin_table_row_form (thispage);

        begin_column ();
        http_send_FS ("<input type=\"text\" name=\"filename\" value=\"");
        http_send (dir.fileName());
        http_send_FS ("\" readonly>");
        end_column ();

        File f = dir.openFile ("r");
  
        if (f)
        {
            sprintf (valbuf, "%d", f.size());
            f.close ();
        }
        else
        {
            strcpy (valbuf, "unknown");
        }
  
        text_rcolumn (valbuf);
        button_column ("remove", "Remove");
        button_column ("show", "Show");
        end_table_row_form ();
    }

    table_trailer ();

    LittleFS.end ();
  
    http_send_FS ("<form method=\"GET\" action=\"/fs\">\r\n"
                  "<button type=\"submit\" name=\"action\" value=\"format\">Format ESP8266 LittleFS</button>"
                  "</form>");

    table_header (update_header_cols, UPDATE_HEADER_COLS);
    table_row_input (thispage, 3, "Update Host", "uphost", update_host, MAX_UPDATE_HOST_LEN);
    table_row_input (thispage, 3, "Update Path", "uppath", update_path, MAX_UPDATE_PATH_LEN);
#if 0
#define MAX_DOWNLOAD_FILENAME 32
    table_row_input (thispage, 3, "Download file", "dwnfil", "", MAX_DOWNLOAD_FILENAME);
#endif
    table_trailer ();

    http_send_FS ("<form method=\"GET\" action=\"/fs\">\r\n"
                  "    <button type=\"submit\" name=\"action\" value=\"download\">Download Icon files</button>\r\n"
                  "</form>\r\n");

    if (hardware_configuration != 0xFFFF && (hardware_configuration & HW_WC_MASK) != HW_UCLOCK)
    {
        http_send_FS ("<form method=\"GET\" action=\"/fs\">"
                      "<select id=\"tablefile\" name=\"tablefile\">");
    
        len = httpclient (update_host, update_path, WC_TABLES_LIST_TXT);
    
        if (len > 0)
        {
            char         fname[MAX_UPDATE_FILENAME_LEN];
            int          ch;
            int          l = 0;

            while (len > 0)
            {
                ch = httpclient_read (&len);

                if (ch < 0)
                {
                    break;                                  // Lesefehler oder Zeitgrenze: *lenp bleibt stehen, sonst dreht die Schleife endlos (L152)
                }

                if (ch != '\r' && ch != '\n' && ch != ' ' && ch != '\t' && l < MAX_UPDATE_FILENAME_LEN - 1)
                {
                    fname[l++] = ch;
                }
                else if (ch == '\n')
                {
                    int show_option;

                    fname[l] = '\0';
                    l = 0;

                    show_option = 1;

                    if (tables_filter)
                    {
                        if (strncmp (fname, tables_filter, strlen (tables_filter)) != 0)
                        {
                            show_option = 0;
                        }
                    }

                    if (show_option)
                    {
                        char * p = tables_fname ();
                        http_send_FS ("<option value=\"");
                        http_send (fname);

                        if (p && ! strcmp (fname, p))
                        {
                            http_send_FS ("\" selected>");
                        }
                        else
                        {
                            http_send_FS ("\">");
                        }
                        http_send (fname);
                        http_send_FS ("</option>\r\n");
                    }
                }
            }

            httpclient_stop ();
            http_flush ();
        }
        else
        {
           Serial.print("Error in HTTP request: ");
           Serial.println(WC_LIST_TXT);
        }

        http_send_FS ("</select>\r\n");

        http_send_FS ("<button type=\"submit\" name=\"action\" value=\"dwntable\">Download layout table</button>"
                      "</form>"
                      "<P>\r\n");
    }

    http_send_FS ("<table style=\"width:auto\">");

    if (fname_icon)
    {
        http_send_FS ("<tr><td>");
        http_send (fname_icon);
        http_send_FS ("</td><td>"
                "<form method='post' action='fs-icon' name='submit' enctype='multipart/form-data' style=\"display:inline\">"
                "<label class='custom-file-upload'><input type='file' name='fileField'>File...</label>&nbsp;"
                "<input type='submit' class='button' name='submit' value='Upload'>"
                "</form></td></tr>"
                );
    }

    if (fname_weather)
    {
        http_send_FS ("<tr><td>");
        http_send (fname_weather);
        http_send_FS ("</td><td>"
                "<form method='post' action='fs-icon-weather' name='submit' enctype='multipart/form-data' style=\"display:inline\">"
                "<label class='custom-file-upload'><input type='file' name='fileField'>File...</label>&nbsp;"
                "<input type='submit' class='button' name='submit' value='Upload'>"
                "</form></td></tr>"
                );
    }

    if (fname_tables)
    {
        http_send_FS ("<tr><td>");
        http_send (fname_tables);
        http_send_FS ("</td><td>"
                "<form method='post' action='fs-tables' name='submit' enctype='multipart/form-data' style=\"display:inline\">"
                "<label class='custom-file-upload'><input type='file' name='fileField'>File...</label>&nbsp;"
                "<input type='submit' class='button' name='submit' value='Upload'>"
                "</form></td></tr>"
                );
    }

    if (hardware_configuration != 0xFFFF && (hardware_configuration & HW_LED_MASK) == HW_LED_TFTLED_RGB_LED && fname_display)
    {
        http_send_FS ("<tr><td>");
        http_send (fname_display);
        http_send_FS ("</td><td>"
                "<form method='post' action='fs-display' name='submit' enctype='multipart/form-data' style=\"display:inline\">"
                "<label class='custom-file-upload'><input type='file' name='fileField'>File...</label>&nbsp;"
                "<input type='submit' class='button' name='submit' value='Upload'>"
                "</form></td></tr>"
                );
    }

    http_send_FS ("</table><P>\r\n");

    http_send_FS ("<B>WordClock PWA (/app)</B><BR>\r\n");
    http_send_FS ("Initial installation of the New App happens via the handoff screen at <a href=\"/app/\">/app</a>. Later app updates are triggered from inside the WordClock PWA itself. The Files page no longer exposes a separate install or download button for that flow.<P>\r\n");

    if (show_fname)
    {
        LittleFS.begin ();
        File fp = LittleFS.open(show_fname, "r");

        if (fp)
        {
            char  b[2];
            int   ch;

            b[1] = '\0';

            http_send (show_fname);
            http_send_FS (":<BR><pre>\r\n");

            while ((ch = fp.read ()) >= 0)
            {
                if (ch == '\r')
                {
                    http_send_FS ("&lt;CR&gt;");
                }
                else if (ch == '\n')
                {
                    http_send_FS ("&lt;LF&gt;\n");
                }
                else
                {
                    b[0] = ch;
                    http_send (b);
                }
            }

            fp.close ();
            http_send_FS ("</pre>\r\n");
        }
        LittleFS.end ();
    }
    end_box ();
    http_trailer ();
    http_flush ();
    return rtc;
}

static const char *
http_get_fs_upload_filename (uint_fast8_t post)
{
    uint_fast16_t led;

    if (hardware_configuration == 0xFFFF)
    {
        return (const char *) 0;
    }

    led = hardware_configuration & HW_LED_MASK;

    switch (hardware_configuration & HW_WC_MASK)
    {
        case HW_WC_24H:
            switch (post)
            {
                case POST_ICON_FILE:         return "wc24h-icon.txt";
                case POST_ICON_WEATHER_FILE: return "wc24h-weather.txt";
                case POST_TABLES_FILE:       return "wc24h-tables-local.txt";
                case POST_DISPLAY_FILE:      return led == HW_LED_TFTLED_RGB_LED ? "wc24h-display-local.txt" : (const char *) 0;
            }
            break;

        case HW_WC_12H:
            switch (post)
            {
                case POST_ICON_FILE:         return "wc12h-icon.txt";
                case POST_ICON_WEATHER_FILE: return "wc12h-weather.txt";
                case POST_TABLES_FILE:       return "wc12h-tables-local.txt";
                case POST_DISPLAY_FILE:      return led == HW_LED_TFTLED_RGB_LED ? "wc12h-display-local.txt" : (const char *) 0;
            }
            break;

        case HW_UCLOCK:
            switch (post)
            {
                case POST_ICON_FILE:         return "uc-icon.txt";
                case POST_ICON_WEATHER_FILE: return "uc-weather.txt";
            }
            break;
    }

    return (const char *) 0;
}

static uint_fast8_t
http_api_fs_upload (uint_fast8_t post)
{
    const char *    filename = http_get_fs_upload_filename (post);
    const char *    target_filename = filename;
    size_t          content_length = 0;
    uint_fast8_t    ok = 0;
    uint32_t        error_code = 0;
    char *          uploaded_name = http_get_param ("filename");

    if (! filename)
    {
        error_code = 1;
    }
    else if ((post == POST_TABLES_FILE && ! http_tables_filename_matches (uploaded_name, filename)) ||
             (post != POST_TABLES_FILE && ! http_filename_matches (uploaded_name, filename)))
    {
        error_code = 5;
    }
    else if (! read_post_headers (&content_length) || content_length == 0)
    {
        error_code = 2;
    }
    else
    {
        LittleFS.begin ();

        if (post == POST_TABLES_FILE)
        {
            target_filename = uploaded_name;
            http_remove_table_family_files (target_filename);
        }

        File f = LittleFS.open (target_filename, "w+");

        if (! f)
        {
            error_code = 3;
        }
        else
        {
            ok = read_request_body_to_file (f, content_length) ? 1 : 0;
            f.close ();

            if (! ok)
            {
                LittleFS.remove (target_filename);
                error_code = 4;
            }
            else if (post == POST_TABLES_FILE)
            {
                tables_init ();
            }
        }

        LittleFS.end ();
    }

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":"));
    http_send (ok ? "true" : "false");

    if (! ok)
    {
        http_send (FS(",\"error\":"));
        http_send (String (error_code).c_str ());
        http_send (FS(",\"detail\":\""));

        switch (error_code)
        {
            case 1: http_send (FS("target unavailable")); break;
            case 2: http_send (FS("invalid request")); break;
            case 3: http_send (FS("open failed")); break;
            case 4: http_send (FS("upload failed")); break;
            case 5: http_send (FS("invalid filename")); break;
            default: http_send (FS("unknown error")); break;
        }

        http_send (FS("\""));
    }

    http_send (FS("}"));
    http_flush ();

    return 0;
}

static uint_fast8_t
http_api_app_file_upload ()
{
    const char *    asset_path = http_get_param ("filename");
    const char *    validated_asset = http_find_app_install_asset (asset_path);
    size_t          content_length = 0;
    uint_fast8_t    ok = 0;
    uint32_t        error_code = 0;
    char            local_filename[64];
    unsigned long   step = 0;
    unsigned long   total = 0;
    const char *    step_param = http_get_param ("step");
    const char *    total_param = http_get_param ("total");

    local_filename[0] = '\0';

    if (*step_param)
    {
        step = strtoul (step_param, (char **) 0, 10);
    }

    if (*total_param)
    {
        total = strtoul (total_param, (char **) 0, 10);
    }

    if (! validated_asset)
    {
        error_code = 1;
    }
    else if (! read_post_headers (&content_length) || content_length == 0)
    {
        error_code = 3;
    }
    else
    {
        const char * encoding = http_get_param ("encoding");
        uint_fast8_t gzip_encoded = (! strcmp (encoding, "gzip")) ? 1 : 0;

        if (! app_asset_storage_filename (validated_asset, gzip_encoded, local_filename, sizeof (local_filename)))
        {
            error_code = 2;
        }
        else
        {
        char stale_filename[64];

        if (app_asset_storage_filename (validated_asset, gzip_encoded ? 0 : 1, stale_filename, sizeof (stale_filename)))
        {
            LittleFS.remove (stale_filename);
        }

        if (step == 1 && total > 0)
        {
            Serial.print (F("APPDL local-install-begin count="));
            Serial.println (total);
        }

        Serial.print (F("APPDL local-install-step "));
        if (step > 0 && total > 0)
        {
            Serial.print (step);
            Serial.print ('/');
            Serial.print (total);
            Serial.print (' ');
        }
        Serial.print (F("remote="));
        Serial.print (validated_asset);
        Serial.print (F(" local="));
        Serial.println (local_filename);

        LittleFS.begin ();

        File f = LittleFS.open (local_filename, "w+");

        if (! f)
        {
            error_code = 4;
        }
        else
        {
            ok = read_request_body_to_file (f, content_length) ? 1 : 0;
            f.close ();

            if (! ok)
            {
                LittleFS.remove (local_filename);
                error_code = 5;
            }
        }

        LittleFS.end ();

        if (ok && total > 0 && step >= total)
        {
            Serial.print (F("APPDL local-install-complete count="));
            Serial.println (total);
        }
        }   // else: app_asset_storage_filename ok
    }   // else: validated_asset && content_length ok

    if (! ok)
    {
        Serial.print (F("APPDL local-install-fail "));
        if (step > 0 && total > 0)
        {
            Serial.print (F("step="));
            Serial.print (step);
            Serial.print ('/');
            Serial.print (total);
            Serial.print (' ');
        }
        if (validated_asset)
        {
            Serial.print (F("remote="));
            Serial.print (validated_asset);
            Serial.print (' ');
        }
        if (local_filename[0])
        {
            Serial.print (F("local="));
            Serial.print (local_filename);
            Serial.print (' ');
        }
        Serial.print (F("error="));
        Serial.println (error_code);
    }

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":"));
    http_send (ok ? "true" : "false");

    if (! ok)
    {
        http_send (FS(",\"error\":"));
        http_send (String (error_code).c_str ());
        http_send (FS(",\"detail\":\""));

        switch (error_code)
        {
            case 1: http_send (FS("invalid filename")); break;
            case 2: http_send (FS("invalid target")); break;
            case 3: http_send (FS("invalid request")); break;
            case 4: http_send (FS("open failed")); break;
            case 5: http_send (FS("upload failed")); break;
            default: http_send (FS("unknown error")); break;
        }

        http_send (FS("\""));
    }

    http_send (FS("}"));
    http_flush ();

    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * HTTP update
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_update (void)
{
    const char *        thispage = "update";
    const char *        update_header_cols[UPDATE_HEADER_COLS]               = { "Name", "Value" };
    const char *        refresh_url = "/update";
    char *              action;
    char *              return_to_app;
    char                flash_stm32_filename[MAX_UPDATE_FILENAME_LEN];
    char                stm32_default_filename[MAX_UPDATE_FILENAME_LEN];
    int                 do_update = 0;
    int                 do_reset = 0;
    int                 flash_rejected = 0;
    STR_VAR *           sv;
    char *              version;
    char *              update_host;
    char *              update_path;
    char                flashsizebuf[32];
    uint32_t            flashsize;
    uint_fast8_t        rtc = 0;

    flash_stm32_filename[0] = '\0';

    sv              = get_strvar (VERSION_STR_VAR);
    version         = sv->str;

    flashsize = ESP.getFlashChipRealSize ();
    sprintf (flashsizebuf, "%d", flashsize);

    action = http_get_param ("action");
    return_to_app = http_get_param ("return_to_app");

    if (! strcmp (return_to_app, "1"))
    {
        refresh_url = "/app/";
    }

    if (*action)
    {
        if (! strcmp (action, "update"))
        {
            do_update = 1;
        }
        else if (! strcmp (action, "saveuphost"))
        {
            set_strvar (UPDATE_HOST_VAR, http_get_param ("uphost"));
        }
        else if (! strcmp (action, "saveuppath"))
        {
            set_strvar (UPDATE_PATH_VAR, http_get_param ("uppath"));
        }
        else if (! strcmp (action, "flash"))
        {
            char * fn = http_get_param ("stm32_filenames");

            if (http_remote_stm32_filename_matches (fn))                // dieselbe Pruefung wie /api/remote_stm32_flash (C9c6/L179)
            {
                strncpy (flash_stm32_filename, fn, MAX_UPDATE_FILENAME_LEN - 1);
                flash_stm32_filename[MAX_UPDATE_FILENAME_LEN - 1] = '\0';
            }
            else
            {
                flash_rejected = 1;
            }
        }
        else if (! strcmp (action, "reset"))
        {
            do_reset = 1;
        }
    }

    sv = get_strvar (UPDATE_HOST_VAR);
    update_host = sv->str;

    if (! update_host[0])
    {
        update_host = (char *) DEFAULT_UPDATE_HOST;
    }

    sv = get_strvar (UPDATE_PATH_VAR);
    update_path = sv->str;

    if (! update_path[0])
    {
        update_path = (char *) DEFAULT_UPDATE_PATH;
    }

    if (do_update)
    {
        http_header ("Update", "40", refresh_url);
    }
    else if (do_reset)
    {
        http_header ("Update", "20", "/");
    }
    else
    {
        http_header ("Update", (const char *) NULL, (const char *) NULL);
    }

    begin_box ("Update");

    if (flashsize >= 4194304)
    {
        if (flash_stm32_filename[0])
        {
            int i;
            bool ok;

            for (i = 0; i < 100; i++)
            {                                                   // send 1000 spaces to force Browser to begin rendering
                http_send ("          "); 
            }

            http_send_FS ("\r\n<P><B>Updating STM32 firmware...<BR>\r\n");
            http_flush ();

            Serial.print ("Flash STM32: http://");
            Serial.print (update_host);
            Serial.print ("/");
            Serial.print (update_path);
            Serial.print ("/");
            Serial.println (flash_stm32_filename);

            delay (200);
            update_progress_begin ("stm32", "starting", "STM32-Update wird vorbereitet.");
            ok = stm32_flash_from_server (update_host, update_path, flash_stm32_filename);
            delay (200);
            Serial.println ("End of flashmode\r\n");
            Serial.flush ();

            if (ok)
            {
                update_progress_complete ("STM32-Flash abgeschlossen. Warte auf Reset.");
            }
            else if (! update_progress.error_code)
            {
                update_progress_fail (1, "STM32-Flash fehlgeschlagen.");
            }

            http_send_FS ("Done. <font color=red>Please Reset your STM32 now!</font></B><BR>\r\n"
                          "<form method=\"GET\" action=\"/update\">\r\n"
                          "<button type=\"submit\" name=\"action\" value=\"reset\">Reset STM32</button>"
                          "</form>"
                          "<P>\r\n");
            end_box ();
            http_trailer ();
            http_flush ();
        }
        else if (do_reset)
        {
            update_progress_state ("reset", "STM32 wird zurückgesetzt.");
            http_send_FS ("<P><B>Resetting STM32, reconnecting in 20 seconds ...</B><BR>\r\n");
            end_box ();
            http_trailer ();
            http_flush ();
            delay (200);
            stm32_reset ();
            update_progress_complete ("STM32 wurde zurückgesetzt.");
        }
        else
        {
            if (do_update)
            {
                char path[MAX_UPDATE_HOST_LEN + MAX_UPDATE_PATH_LEN + MAX_UPDATE_FILENAME_LEN + 3];
    
                sprintf (path, "/%s/%s", update_path, ESP_WORDCLOCK_BIN);
                update_progress_begin ("esp", "starting", "ESP-Firmware-Update wird gestartet.");
    
                http_send_FS ("<P><B>Updating ESP firmware '");
                http_send (path);
                http_send_FS ("', reconnecting in 40 seconds ...</B></BR>\r\n");
                end_box ();
                http_trailer ();
                http_flush ();
                while (http_client.available())  // firefox claims about connection reset, if we do not read all characters
                {
                    http_client.read();
                }
                http_client.stop();
                delay (200);
                update_progress_state ("reconnect_wait", "ESP-Firmware wird geladen. Gerät startet danach neu.");
    
                t_httpUpdate_return ret = ESPhttpUpdate.update (http_client, update_host, 80, path);
    
                switch(ret)
                {
                    case HTTP_UPDATE_FAILED:
                        Serial.println("HTTP update: failed");
                        update_progress_fail (ESPhttpUpdate.getLastError (), ESPhttpUpdate.getLastErrorString ().c_str ());
                        break;
    
                    case HTTP_UPDATE_NO_UPDATES:
                        Serial.println("HTTP update: no updates");
                        update_progress_complete ("Keine neue ESP-Firmware verfügbar.");
                        break;
    
                    case HTTP_UPDATE_OK:
                        Serial.println("HTTP update: ok");    // will be never called, because we are rebooting
                        break;
                }
            }
            else
            {
                char    new_esp_version[16];
                char    new_wc_version[16];
                int     len;
                const char * filter;

                http_build_stm32_default_filename (stm32_default_filename, sizeof (stm32_default_filename), &filter);
    
                new_esp_version[0] = '\0';
                new_wc_version[0] = '\0';
    
                len = httpclient (update_host, update_path, ESP_WORDCLOCK_TXT);
    
                if (len > 0)
                {
                    int ch;
                    int l = 0;
    
                    while (len > 0)
                    {
                        ch = httpclient_read (&len);

                        if (ch < 0)
                        {
                            break;                                  // Lesefehler oder Zeitgrenze: *lenp bleibt stehen, sonst dreht die Schleife endlos (L152)
                        }
    
                        if (ch != '\r' && ch != '\n' && l < 16 - 1)
                        {
                            new_esp_version[l++] = ch;
                        }
                    }
    
                    httpclient_stop ();
                    new_esp_version[l] = '\0';
                }
                else
                {
                   Serial.print("Error in HTTP request: ");
                   Serial.println(ESP_WORDCLOCK_TXT);
                }
    
                len = httpclient (update_host, update_path, RELEASENOTE_HTML);
    
                if (len > 0)
                {
                    char linebuf[128];
                    int l = 0;
                    int ch;
    
                    while (len > 0)
                    {
                        ch = httpclient_read (&len);

                        if (ch < 0)
                        {
                            break;                                  // Lesefehler oder Zeitgrenze: *lenp bleibt stehen, sonst dreht die Schleife endlos (L152)
                        }
    
                        if (l < 128 - 1 && ch > 0)
                        {
                            linebuf[l++] = ch;
                        }
    
                        if (ch == '\n')
                        {
                            linebuf[l] = '\0';
                            l = 0;
                            http_send (linebuf);
                        }
                    }
    
                    httpclient_stop ();
                }
                else
                {
                   Serial.print("Error in HTTP request: ");
                   Serial.println(RELEASENOTE_HTML);
                }
    
                len = httpclient (update_host, update_path, WC_TXT);
    
                if (len > 0)
                {
                    int ch;
                    int l = 0;
    
                    while (len > 0)
                    {
                        ch = httpclient_read (&len);

                        if (ch < 0)
                        {
                            break;                                  // Lesefehler oder Zeitgrenze: *lenp bleibt stehen, sonst dreht die Schleife endlos (L152)
                        }
    
                        if (ch != '\r' && ch != '\n' && l < 16 - 1)
                        {
                            new_wc_version[l++] = ch;
                        }
                    }
    
                    httpclient_stop ();
                    new_wc_version[l] = '\0';
                }
                else
                {
                   Serial.print("Error in HTTP request: ");
                   Serial.println(WC_TXT);
                }
    
                table_header (update_header_cols, UPDATE_HEADER_COLS);
                table_row_input (thispage, 3, "Update Host", "uphost", update_host, MAX_UPDATE_HOST_LEN, 0);
                table_row_input (thispage, 3, "Update Path", "uppath", update_path, MAX_UPDATE_PATH_LEN, 0);
                table_trailer ();
    
                http_send_FS ("<table><tr><td>ESP8266 flash size</td><td>");
                http_send (flashsizebuf);
                http_send_FS ("</td></tr><tr><td>ESP firmware version</td><td>");
                http_send (ESP_VERSION);
                http_send_FS ("</td></tr>\r\n"
                              "<tr><td>ESP firmware available</td><td>");
                http_send (new_esp_version);
                http_send_FS ("</td></tr></table>\r\n");
    
                http_send_FS ("<form method=\"GET\" action=\"/update\">\r\n"
                              "    <button type=\"submit\" name=\"action\" value=\"update\">Update ESP Firmware</button>"
                              "</form>"
                              "<P>"
                              "<table>"
                              "<tr>"
                              "<td>WordClock firmware version</td>"
                              "<td>");

                http_send (version);
                http_send_FS ("</td>"
                              "</tr>"
                              "<tr>"
                              "<td>WordClock firmware available</td>"
                              "<td>");
                http_send (new_wc_version);
                http_send_FS ("</td>"
                              "</tr>"
                              "</table>"
                              "<form method=\"GET\" action=\"/update\">"
                              "<select id=\"stm32_filenames\" name=\"stm32_filenames\">");
    
                len = httpclient (update_host, update_path, WC_LIST_TXT);
    
                if (len > 0)
                {
                    char fname[MAX_UPDATE_FILENAME_LEN];
                    int ch;
                    int l = 0;

                    if (! filter)
                    {
                        http_send_FS ("<option value=\"\" selected>&lt;unknown&gt;</option>\r\n");
                    }

                    while (len > 0)
                    {
                        ch = httpclient_read (&len);

                        if (ch < 0)
                        {
                            break;                                  // Lesefehler oder Zeitgrenze: *lenp bleibt stehen, sonst dreht die Schleife endlos (L152)
                        }
    
                        if (ch != '\r' && ch != '\n' && ch != ' ' && ch != '\t' && l < MAX_UPDATE_FILENAME_LEN - 1)
                        {
                            fname[l++] = ch;
                        }
                        else if (ch == '\n')
                        {
                            int show_option;

                            fname[l] = '\0';
                            l = 0;

                            show_option = http_remote_stm32_filename_matches (fname);   // dieselbe Pruefung wie Flashzweig und API (C9c6, L272)

                            if (show_option)
                            {
                                // A6 (Review F.6): fname kommt aus wc-list.txt vom Update-Server - nicht
                                // vertrauenswuerdig (konfigurierbarer Host, HTTP). Maskiert wie die API-Liste.
                                http_send_FS ("<option value=\"");
                                http_send_xml_escaped (fname);
        
                                if (! strcmp (fname, stm32_default_filename))
                                {
                                    http_send_FS ("\" selected>");
                                }
                                else
                                {
                                    http_send_FS ("\">");
                                }
        
                                http_send_xml_escaped (fname);
                                http_send_FS ("</option>\r\n");
                            }
                        }
                    }
    
                    httpclient_stop ();
                    http_flush ();
                }
                else
                {
                   Serial.print("Error in HTTP request: ");
                   Serial.println(WC_LIST_TXT);
                }
                http_send_FS ("</select>\r\n");
    
                http_send_FS ("<button type=\"submit\" name=\"action\" value=\"flash\">Flash STM32</button>"
                              "</form>"
                              "<P>\r\n");

                if (flash_rejected)
                {
                    http_send_FS ("<P><font color=red><B>Abgewiesen: Die STM32-Datei passt nicht zur erkannten Hardware. Nicht geflasht.</B></font><BR>\r\n");
                }

                /* 65535 oder unbekannter Typ: Liste leer, Rueckweg nennen (AKF.2). Der Reset holt die
                 * Hardwarekennung zurueck; tools/flash-stm.sh hilft hier NICHT, es bricht bei 65535 ab
                 * (Review F.6, A1). Der Weg ueber "Local Update" (flash_stm32_local) prueft keinen
                 * Dateinamen und bleibt deshalb offen - der Endpunkt /api/local_stm32_upload dagegen
                 * weist bei 65535 ab.
                 */
                if (! filter)
                {
                    http_send_FS ("<P><font color=red>Hardware nicht erkannt - keine STM32-Datei angeboten. "
                                  "Zuerst den STM zur&uuml;cksetzen und die Seite neu laden. "
                                  "Hilft das nicht: unter &bdquo;Local Update&ldquo; eine .hex hochladen.</font><BR>\r\n"
                                  "<form method=\"GET\" action=\"/update\">\r\n"
                                  "<button type=\"submit\" name=\"action\" value=\"reset\">Reset STM32</button>"
                                  "</form>"
                                  "<P>\r\n");
                }
            }

            end_box ();
            http_trailer ();
            http_flush ();
        }
    }
    else
    {
        http_send_FS ("<font color=red><B>Flash size of ESP8266 is too small for Update over OTA</B></font><BR>\r\n");
        end_box ();
        http_trailer ();
        http_flush ();
    }

    return rtc;
}

/* Laenge einer gueltigen UTF-8-Sequenz an dieser Stelle, sonst 0. Strikt inklusive
 * Ueberlang-, Surrogat- und Bereichspruefung: Ein halbes oder krummes Zeichen im
 * Attributwert laesst den DOMParser der PWA scheitern, und danach ist der Wert ueber
 * die PWA nicht mehr zu korrigieren (L46). Greift auch fuer Quellen ausserhalb der
 * Setter, etwa SSIDs aus dem WLAN-Scan.
 */
static unsigned int
utf8_sequence_len (const char * str, unsigned int len, unsigned int pos)
{
    unsigned char  c0 = (unsigned char) str[pos];
    unsigned int   need;
    unsigned char  c1_min = 0x80;
    unsigned char  c1_max = 0xBF;
    unsigned int   i;

    if (c0 < 0xC2)                                                                  // 0x80..0xC1: Folgebyte ohne Start oder ueberlang
    {
        return 0;
    }
    else if (c0 <= 0xDF)
    {
        need = 2;
    }
    else if (c0 <= 0xEF)
    {
        need = 3;

        if (c0 == 0xE0)                                                             // ueberlange 3-Byte-Form
        {
            c1_min = 0xA0;
        }
        else if (c0 == 0xED)                                                        // UTF-16-Surrogate
        {
            c1_max = 0x9F;
        }
    }
    else if (c0 <= 0xF4)
    {
        need = 4;

        if (c0 == 0xF0)                                                             // ueberlange 4-Byte-Form
        {
            c1_min = 0x90;
        }
        else if (c0 == 0xF4)                                                        // oberhalb U+10FFFF
        {
            c1_max = 0x8F;
        }
    }
    else
    {
        return 0;
    }

    if (pos + need > len)
    {
        return 0;
    }

    for (i = 1; i < need; i++)
    {
        unsigned char ci = (unsigned char) str[pos + i];

        if (ci < ((i == 1) ? c1_min : 0x80) || ci > ((i == 1) ? c1_max : 0xBF))
        {
            return 0;
        }
    }

    return need;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * http_escape_text () - Fremdtext maskiert in einen Puffer schreiben, ganz ohne String
 *
 * Das Ergebnis landet in einem Puffer des Aufrufers, der auf dem Stack liegen darf -
 * keine einzige Anforderung am Haufen. Das ersetzt sanitize_xml_string () und
 * sanitize_json_string (), die ihr Ergebnis zeichenweise als String aufbauten und damit
 * den groessten zusammenhaengenden Block zerlegten (BEFUNDE.md L175).
 *
 * DREI MODI, weil es in dieser Datei drei verschiedene Ziele gibt:
 *
 * HTTP_ESCAPE_XML    Zeichen fuer Zeichen identisch zu sanitize_xml_string ():
 *                    & < > " ' werden Entities, ungueltige UTF-8-Folgen und in XML 1.0
 *                    verbotene Steuerzeichen werden '?'. \t \n \r bleiben stehen.
 * HTTP_ESCAPE_JSON   Zeichen fuer Zeichen identisch zu sanitize_json_string ():
 *                    \\ " \r \n \t werden mit Rueckstrich maskiert, alles andere bleibt
 *                    roh. Bewusst OHNE UTF-8-Pruefung - die Ausgabe soll sich an den
 *                    bestehenden Aufrufstellen nicht aendern.
 * HTTP_ESCAPE_JSON_LOG  Nur /api/stm32_log (C23/L207). Wie HTTP_ESCAPE_JSON, dazu
 *                    zweierlei, damit die Antwort gueltiges JSON in UTF-8 ist: Steuer-
 *                    zeichen unter 0x20 ausser \r \n \t werden \u00XX, und ein Byte ab
 *                    0x80, das KEINE gueltige UTF-8-Folge beginnt, gilt als ISO-8859-1
 *                    und wird in seine Zwei-Byte-Form gewandelt. Gueltige UTF-8-Folgen
 *                    bleiben stehen: Im Ring liegen neben den STM-Zeilen (ISO-8859-1)
 *                    auch ESP-Zeilen, und var_cmd_reject () (vars.cpp) zitiert dort
 *                    Kommandotext, der UTF-8 sein kann - pauschal gewandelt waere der
 *                    doppelt kodiert.
 * HTTP_ESCAPE_SCAN   Die Mischform, die /api/network_scan seit jeher liefert: XML-
 *                    Entities innerhalb eines JSON-Strings. Dazu zwei Ergaenzungen, die
 *                    nur dort greifen, wo die Antwort bisher ohnehin unbrauchbar war -
 *                    der Rueckstrich wird als \\ maskiert (eine SSID mit Rueckstrich
 *                    liefert sonst ungueltiges JSON, und die PWA verliert die KOMPLETTE
 *                    Netzliste), und Steuerzeichen werden ausnahmslos '?', auch \t \n \r,
 *                    die ein JSON-String roh nicht erlaubt.
 *
 * Rueckgabewert ist die Zahl der geschriebenen Zeichen ohne die Null. Passt eine
 * Maskierung oder eine UTF-8-Folge nicht mehr vollstaendig in den Puffer, bricht die
 * Schleife VOR ihr ab - eine halbe Entity waere schlimmer als ein kuerzerer Name. Wie
 * viele QUELLzeichen dabei verbraucht wurden, meldet *src_used; darauf setzt
 * http_send_escaped () auf, um beliebig lange Texte in Stuecken zu stroemen.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static size_t
http_escape_text (const char * src, unsigned int src_len, char * out, size_t out_len, uint_fast8_t mode, unsigned int * src_used)
{
    unsigned int    i = 0;
    size_t          o = 0;

    if (! out || out_len == 0)
    {
        if (src_used)
        {
            *src_used = 0;
        }

        return 0;
    }

    while (src && i < src_len)
    {
        unsigned char   uc  = (unsigned char) src[i];
        const char *    esc = (const char *) 0;
        unsigned int    seq = 1;
        char            conv[8];

        if (mode == HTTP_ESCAPE_JSON || mode == HTTP_ESCAPE_JSON_LOG)
        {
            switch (uc)
            {
                case '\\': esc = "\\\\"; break;
                case '"':  esc = "\\\""; break;
                case '\r': esc = "\\r";  break;
                case '\n': esc = "\\n";  break;
                case '\t': esc = "\\t";  break;
                default:   break;
            }

            if (! esc && mode == HTTP_ESCAPE_JSON_LOG)                              // C23
            {
                if (uc < 0x20)
                {
                    snprintf (conv, sizeof (conv), "\\u%04x", uc);                  // roh waere ungueltiges JSON
                    esc = conv;
                }
                else if (uc >= 0x80)
                {
                    seq = utf8_sequence_len (src, src_len, i);

                    if (! seq)                                                      // kein UTF-8: ISO-8859-1-Byte, zwei Byte UTF-8
                    {
                        conv[0] = (char) (0xC0 | (uc >> 6));
                        conv[1] = (char) (0x80 | (uc & 0x3F));
                        conv[2] = '\0';
                        esc = conv;
                        seq = 1;
                    }
                }
            }
        }
        else if (uc >= 0x80)
        {
            seq = utf8_sequence_len (src, src_len, i);

            if (! seq)
            {
                esc = "?";                                                          // ungueltige Sequenz sichtbar ersetzen statt durchreichen (L46)
                seq = 1;
            }
        }
        else if (uc < 0x20 && (mode == HTTP_ESCAPE_SCAN || (uc != '\t' && uc != '\n' && uc != '\r')))
        {
            esc = "?";                                                              // in XML 1.0 verbotene Steuerzeichen; im SCAN-Modus ausnahmslos alle
        }
        else
        {
            switch (uc)
            {
                case '&':  esc = "&amp;";  break;
                case '<':  esc = "&lt;";   break;
                case '>':  esc = "&gt;";   break;
                case '"':  esc = "&quot;"; break;
                case '\'': esc = "&apos;"; break;
                case '\\':
                    if (mode == HTTP_ESCAPE_SCAN)
                    {
                        esc = "\\\\";
                    }
                    break;
                default:   break;
            }
        }

        if (esc)
        {
            size_t esc_len = strlen (esc);

            if (o + esc_len >= out_len)
            {
                break;
            }

            memcpy (out + o, esc, esc_len);
            o += esc_len;
            i++;
        }
        else
        {
            if (o + seq >= out_len)
            {
                break;
            }

            memcpy (out + o, src + i, seq);
            o += seq;
            i += seq;
        }
    }

    out[o] = '\0';

    if (src_used)
    {
        *src_used = i;                                                              // wie viele QUELLzeichen verbraucht wurden
    }

    return o;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * http_send_escaped () - Fremdtext maskiert in die laufende Antwort stroemen, ohne Haufen
 *
 * Ersetzt das Muster "http_send (sanitize_xxx_string (x).c_str ())", das an 28 Stellen
 * stand. Der Puffer liegt auf dem Stack und wird stueckweise geleert; eine Maskierung
 * oder eine UTF-8-Folge wird nie zerschnitten, weil http_escape_text () vor einer nicht
 * mehr passenden Folge abbricht und meldet, wie weit es gekommen ist.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
http_send_escaped (const char * s, uint_fast8_t mode)
{
    char            buf[136];                                                       // laengste Maskierung 6 Byte, laengste UTF-8-Folge 4
    unsigned int    len;
    unsigned int    pos = 0;

    if (! s)
    {
        return;
    }

    len = (unsigned int) strlen (s);

    while (pos < len)
    {
        unsigned int    used = 0;

        http_escape_text (s + pos, len - pos, buf, sizeof (buf), mode, &used);
        http_send (buf);

        if (! used)
        {
            break;                                                                  // kann nicht vorkommen, verhindert aber jede Endlosschleife
        }

        pos += used;
    }
}

static void
http_send_xml_escaped (const char * s)
{
    http_send_escaped (s, HTTP_ESCAPE_XML);
}

/* Rueckgabewert: > 0 heisst, der Server hat vollstaendig geantwortet; <= 0 heisst
 * Fehlschlag (keine Antwort, Status != 200, oder Abbruch beim Lesen). Frueher void -
 * damit war von aussen nicht zu unterscheiden, ob der Server eine leere Zeile geliefert
 * hat oder gar nicht geantwortet hat. Genau diese Unterscheidung braucht das
 * Sperrfenster in http_api_update_status () (L161).
 *
 * ACHTUNG, hier liegt eine Falle: len wird von httpclient_read () HERUNTERGEZAEHLT und
 * ist nach einem erfolgreichen Durchlauf 0. len selbst zurueckzugeben haette die
 * Bedeutung genau umgedreht. Deshalb ein eigener Rueckgabewert.
 */
static int
http_fetch_remote_line (const char * host, const char * path, const char * filename, char * buffer, size_t buffer_len)
{
    int len;
    int rtc;
    int l = 0;

    if (buffer_len == 0)
    {
        return -1;
    }

    buffer[0] = '\0';
    len = httpclient (host, path, filename);
    rtc = len;

    if (len > 0)
    {
        int ch;

        while (len > 0)
        {
            ch = httpclient_read (&len);

            if (ch < 0)
            {
                l = 0;                                  // Lesefehler oder Zeitgrenze (L152): lieber LEER zurueckgeben als
                break;                                  // halb. Eine abgeschnittene Versionsnummer wie "3.2." sieht sonst
            }                                           // aus wie eine echte und meldet ein Update, das es nicht gibt.

            if (ch != '\r' && ch != '\n' && l < (int) buffer_len - 1)
            {
                buffer[l++] = ch;
            }
        }

        httpclient_stop ();

        if (len > 0)
        {
            rtc = -1;                               // Rest uebrig: die Schleife ist ueber ch < 0 ausgestiegen
        }
    }

    buffer[l] = '\0';

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * http_json_send_remote_text_field () - Text vom Update-Server als JSON-Feld ausgeben,
 * ohne ihn vorher vollstaendig im Speicher zu sammeln
 *
 * Ersetzt das fruehere Paar http_fetch_remote_text () + sanitize_json_string (). Das
 * hielt denselben Inhalt ZWEIMAL im Heap: einmal den rohen String (bis 3072 Byte) und
 * einmal die maskierte Kopie, die sanitize_json_string () zeichenweise aufbaut und
 * dabei mehrfach umkopiert. Beide lagen gleichzeitig vor, und der rohe String lebte vom
 * Kopf von http_api_update_status () bis zur Ausgabe des Feldes, also ueber die GANZE
 * Antwort. Spitzenbedarf ueber 6 KB gegen 5'400 Byte freien Heap und 4'648 Byte
 * groessten zusammenhaengenden Block (L134).
 *
 * Genau das war die Ursache der verstuemmelten Antworten: lwIP kopiert jeden Block in
 * frisch angeforderte pbufs (TCP_WRITE_FLAG_COPY, weil _sync aus ist), bekam keinen
 * Speicher mehr, tcp_write () antwortete mit ERR_MEM, und _write_from_source () gab
 * nach 5000 ms ohne Fortschritt auf - am Geraet gemessen 7012 statt 8084 Byte in 11 von
 * 12 Abrufen. Die Pruefung in http_flush () macht diesen Verlust seit ESP 3.2.14
 * sichtbar; sie verhindert ihn nicht. Der Endpunkt soll aber GELINGEN, nicht ehrlich
 * scheitern - deshalb hier die Ursache statt nur die Meldung.
 *
 * Der Zeilenpuffer liegt auf dem Stack und wird nach jeweils rund 128 Zeichen ueber
 * http_send () geleert. Keine einzige Heap-Anforderung, und der 3072-Byte-Hoechstwert
 * wirkt weiterhin auf die Zahl der gelesenen QUELLzeichen, nicht auf die maskierte
 * Laenge - wie zuvor bei result.length () < max_len.
 *
 * MASKIERUNG: Zeichen fuer Zeichen identisch zu sanitize_json_string () - '\\', '"',
 * '\r', '\n', '\t', alles andere roh. Das ist hier wichtiger als es aussieht: Die
 * Maskierung ist ZUSTANDSLOS, jedes Quellzeichen wird fuer sich entschieden. Deshalb
 * kann eine Escape-Folge an einer Puffergrenze nicht zerfallen - sie wird immer als
 * Ganzes geschrieben, und wenn sie nicht mehr passt, geht der Puffer vorher hinaus.
 * Ebenso unkritisch ist eine Mehrbyte-UTF-8-Folge: Jedes ihrer Bytes landet im
 * default-Zweig unveraendert im Strom und in derselben Reihenfolge; wo die
 * Puffergrenze faellt, sieht der Empfaenger nicht.
 *
 * EINZIGER Unterschied zum frueheren Verhalten: ein 0x00 im Fremdtext. Vorher landete
 * es im String, und http_send () brach die Ausgabe dort per strlen () ab - die Release
 * Notes waren ab dieser Stelle weg. Hier wird das Byte uebersprungen, zaehlt aber wie
 * zuvor gegen max_len. Ein rohes 0x00 in einem JSON-String waere ohnehin ungueltig.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
/* Rueckgabewert wie bei http_fetch_remote_line (): > 0 bei Antwort des Servers, sonst
 * <= 0. Das Feld wird in jedem Fall ausgegeben, notfalls leer - die Antwortform bleibt
 * gleich. Auch hier gilt: len wird heruntergezaehlt und taugt nicht als Rueckgabewert,
 * zumal die Schleife bei max_len absichtlich mit len > 0 endet.
 */
static int
http_json_send_remote_text_field (const char * key, const char * host, const char * path, const char * filename, size_t max_len)
{
    char    buf[132];                                   // 128 Nutzzeichen plus Reserve fuer eine Escape-Folge und die Null
    size_t  idx = 0;
    size_t  taken = 0;                                  // gelesene Quellzeichen, Vergleichsgroesse fuer max_len
    int     len;
    int     rtc;

    http_json_send_field_prefix (key);
    http_send (FS("\""));

    len = httpclient (host, path, filename);
    rtc = len;

    if (len > 0)
    {
        while (len > 0 && taken < max_len)
        {
            int             ch = httpclient_read (&len);
            const char *    esc;
            char            raw[2];
            size_t          esc_len;

            if (ch < 0)
            {
                break;                                  // Lesefehler oder Zeitgrenze (L152): *lenp bleibt stehen, sonst dreht die Schleife endlos
            }

            taken++;

            if (ch == 0)
            {
                continue;                               // siehe Kommentar oben
            }

            switch (ch)
            {
                case '\\': esc = "\\\\"; break;
                case '"':  esc = "\\\""; break;
                case '\r': esc = "\\r";  break;
                case '\n': esc = "\\n";  break;
                case '\t': esc = "\\t";  break;
                default:
                    raw[0] = (char) ch;
                    raw[1] = '\0';
                    esc    = raw;
                    break;
            }

            esc_len = strlen (esc);

            if (idx + esc_len >= sizeof (buf))          // Escape-Folge nie zerschneiden
            {
                buf[idx] = '\0';
                http_send (buf);
                idx = 0;
            }

            memcpy (buf + idx, esc, esc_len);
            idx += esc_len;
        }

        if (idx > 0)
        {
            buf[idx] = '\0';
            http_send (buf);
        }

        /* Wie zuvor: Bei Erreichen von max_len wird der Rest NICHT nachgelesen, die
         * Verbindung wird einfach geschlossen.
         */
        httpclient_stop ();
    }

    http_send (FS("\""));

    return rtc;
}

static void
http_build_stm32_default_filename (char * stm32_default_filename, size_t max_len, const char ** filter)
{
    stm32_default_filename[0] = '\0';
    *filter = (const char *) NULL;

    if (hardware_configuration == 0xFFFF || max_len == 0)
    {
        return;
    }

    switch (hardware_configuration & HW_WC_MASK)
    {
        case HW_WC_24H:                       strcat (stm32_default_filename, "wc24h-");          break;
        case HW_WC_12H:                       strcat (stm32_default_filename, "wc12h-");          break;
        case HW_UCLOCK:                       strcat (stm32_default_filename, "uc-");             break;
    }

    switch (hardware_configuration & HW_STM32_MASK)
    {
        case HW_STM32_F103C8:                 strcat (stm32_default_filename, "stm32f103-");        *filter = "stm32f103-"; break;
        case HW_STM32_F401RE:                 strcat (stm32_default_filename, "stm32f401-");        *filter = "stm32f401-"; break;
        case HW_STM32_F411RE:                 strcat (stm32_default_filename, "stm32f411-");        *filter = "stm32f411-"; break;
        case HW_STM32_F446RE:                 strcat (stm32_default_filename, "stm32f446-");        *filter = "stm32f446-"; break;
        case HW_STM32_F407VE:                 strcat (stm32_default_filename, "stm32f407-");        *filter = "stm32f407-"; break;
        case HW_STM32_F401CC:
        {
            switch (hardware_configuration & HW_OSC_FREQUENCY_MASK)
            {
                case HW_OSC_FREQUENCY_8MHZ:   strcat (stm32_default_filename, "stm32f401cc-8-");    *filter = "stm32f401cc-8-"; break;
                case HW_OSC_FREQUENCY_25MHZ:  strcat (stm32_default_filename, "stm32f401cc-25-");   *filter = "stm32f401cc-25-"; break;
            }
            break;
        }
        case HW_STM32_F411CE:
        {
            switch (hardware_configuration & HW_OSC_FREQUENCY_MASK)
            {
                case HW_OSC_FREQUENCY_8MHZ:   strcat (stm32_default_filename, "stm32f411ce-8-");    *filter = "stm32f411ce-8-"; break;
                case HW_OSC_FREQUENCY_25MHZ:  strcat (stm32_default_filename, "stm32f411ce-25-");   *filter = "stm32f411ce-25-"; break;
            }
            break;
        }
    }

    switch (hardware_configuration & HW_LED_MASK)
    {
        case HW_LED_WS2812_GRB_LED:     strcat (stm32_default_filename, "ws2812-grb.hex");  break;
        case HW_LED_WS2812_RGB_LED:     strcat (stm32_default_filename, "ws2812-rgb.hex");  break;
        case HW_LED_APA102_RGB_LED:     strcat (stm32_default_filename, "apa102-grb.hex");  break;
        case HW_LED_SK6812_RGB_LED:     strcat (stm32_default_filename, "sk6812-rgb.hex");  break;
        case HW_LED_SK6812_RGBW_LED:    strcat (stm32_default_filename, "sk6812-rgbw.hex"); break;
        case HW_LED_TFTLED_RGB_LED:     strcat (stm32_default_filename, "tftled-rgb.hex");  break;
    }
}

static int
http_send_settings_xml (const char * header)
{
    char          buff[255];
    int           i;
    uint_fast8_t  ui;
    TM *          tm;

    http_send(header ? header : FS("HTTP/1.0 200 OK\r\n\r\n"));
    http_send(FS("<settings>"));

    // num vars
    for (i = 0; i < MAX_NUM_VARIABLES; i++)
    {
        sprintf(buff, FS("<numvar idx=\"%d\" value=\"%d\" />"), i, numvars[i]);
        http_send(buff);
    }

    // string vars
    for (i = 0; i < MAX_STR_VARIABLES; i++)
    {
        /* Der maskierte Wert wird GESTROEMT statt in buff formatiert (L175). Das spart
          * nicht nur den String, es beseitigt auch einen stillen Ueberlauf: Ein Wert von
          * 63 Zeichen - update_host, update_path, reset_cause - wird im schlimmsten Fall
          * zu 378 Zeichen maskiert und passte nie in buff[255].
          */
        snprintf(buff, sizeof (buff), FS("<strvar idx=\"%d\" value=\""), i);
        http_send(buff);
        http_send_xml_escaped(strvars[i].str);
        http_send(FS("\" />"));
    }

    // tm vars
    for (i = 0; i < MAX_TM_VARIABLES; i++)
    {
        tm = get_tm_var ((TM_VARIABLE) i);

        if (tm)
        {
            sprintf (buff, FS("<tmvar idx=\"%d\" year=\"%d\" month=\"%d\" day=\"%d\" hour=\"%d\" minute=\"%d\" second=\"%d\" wday=\"%d\" />"),
                i,
                tm->tm_year + 1900,
                tm->tm_mon + 1,
                tm->tm_mday,
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec,
                tm->tm_wday);
            http_send (buff);
        }
    }

    // colors
    for (i = 0; i < MAX_DSP_COLOR_VARIABLES; i++)
    {
        sprintf(buff, FS("<dspcolor idx=\"%d\" red=\"%d\" green=\"%d\" blue=\"%d\" white=\"%d\" />"), i, dspcolorvars[i].red, dspcolorvars[i].green, dspcolorvars[i].blue, dspcolorvars[i].white);
        http_send(buff);
    }

    // num8 arrays
    for (i = 0; i < MAX_BRIGHTNESS + 1; i++)
    {
        sprintf(buff, FS("<num8array var=\"%d\" idx=\"%d\" value=\"%d\" />"), DISPLAY_DIMMED_DISPLAY_COLORS, i, get_num8_array(DISPLAY_DIMMED_DISPLAY_COLORS, i));
        http_send(buff);
    }

    for (i = 0; i < MAX_BRIGHTNESS + 1; i++)
    {
        sprintf(buff, FS("<num8array var=\"%d\" idx=\"%d\" value=\"%d\" />"), DISPLAY_DIMMED_AMBILIGHT_COLORS, i, get_num8_array(DISPLAY_DIMMED_AMBILIGHT_COLORS, i));
        http_send(buff);
    }

    // display modes
    for (ui = 0; ui < display_modes_count; ui++)
    {
        snprintf(buff, sizeof (buff), FS("<dispmode idx=\"%d\" name=\""), ui);
        http_send(buff);
        http_send_xml_escaped(tbl_modes[ui].description);
        http_send(FS("\" />"));
    }

    // display animations
    for (i = 0; i < max_display_animation_variables; i++)
    {
        snprintf(buff, sizeof (buff), FS("<dispanim idx=\"%d\" name=\""), i);
        http_send(buff);
        http_send_xml_escaped(displayanimationvars[i].name);
        snprintf(buff, sizeof (buff), FS("\" dcl=\"%d\" def_dcl=\"%d\" flags=\"%d\"/>"),
            displayanimationvars[i].deceleration,
            displayanimationvars[i].default_deceleration,
            displayanimationvars[i].flags);
        http_send(buff);
    }

    // color animations
    for (i = 0; i < MAX_COLOR_ANIMATION_VARIABLES; i++)
    {
        snprintf(buff, sizeof (buff), FS("<coloranim idx=\"%d\" name=\""), i);
        http_send(buff);
        http_send_xml_escaped(coloranimationvars[i].name);
        snprintf(buff, sizeof (buff), FS("\" dcl=\"%d\" def_dcl=\"%d\" flags=\"%d\"/>"),
            coloranimationvars[i].deceleration,
            coloranimationvars[i].default_deceleration,
            coloranimationvars[i].flags);
        http_send(buff);
    }

    // ambilight modes
    for (i = 0; i < MAX_AMBILIGHT_MODE_VARIABLES; i++)
    {
        snprintf(buff, sizeof (buff), FS("<almode idx=\"%d\" name=\""), i);
        http_send(buff);
        http_send_xml_escaped(ambilightmodevars[i].name);
        snprintf(buff, sizeof (buff), FS("\" dcl=\"%d\" def_dcl=\"%d\" flags=\"%d\"/>"),
            ambilightmodevars[i].deceleration,
            ambilightmodevars[i].default_deceleration,
            ambilightmodevars[i].flags);
        http_send(buff);
    }

    // night times
    for (i = 0; i < MAX_NIGHT_TIME_VARIABLES; i++)
    {
        sprintf(buff, FS("<nighttime idx=\"%d\" minutes=\"%d\" flags=\"%d\"/>"), i, nighttimevars[i].minutes, nighttimevars[i].flags);
        http_send(buff);
    }

    // ambilight night times
    for (i = 0; i < MAX_NIGHT_TIME_VARIABLES; i++)
    {
        sprintf(buff, FS("<ambinighttime idx=\"%d\" minutes=\"%d\" flags=\"%d\"/>"), i, ambilightnighttimevars[i].minutes, ambilightnighttimevars[i].flags);
        http_send(buff);
    }

    // alarm times
    for (i = 0; i < MAX_ALARM_TIME_VARIABLES; i++)
    {
        sprintf(buff, FS("<alarmtime idx=\"%d\" minutes=\"%d\" flags=\"%d\"/>"), i, alarmtimevars[i].minutes, alarmtimevars[i].flags);
        http_send(buff);
    }

    // overlays
    for (i = 0; i < MAX_OVERLAYS; i++)
    {
        snprintf(buff, sizeof (buff), FS("<overlay idx=\"%d\" type=\"%d\" interval=\"%d\" duration=\"%d\" date_code=\"%d\" date_start=\"%d\" days=\"%d\" flags=\"%d\" text=\""),
            i,
            overlays[i].type,
            overlays[i].interval,
            overlays[i].duration,
            overlays[i].date_code,
            overlays[i].date_start,
            overlays[i].days,
            overlays[i].flags);
        http_send(buff);
        http_send_xml_escaped(overlays[i].text);
        http_send(FS("\"/>"));
    }

    http_send(FS("</settings>"));
    http_flush();

    return 0;
}

static int
http_get_settings()
{
    return http_send_settings_xml (FS("HTTP/1.0 200 OK\r\n\r\n"));
}

static int
http_api_settings_xml ()
{
    return http_send_settings_xml (FS("HTTP/1.0 200 OK\r\nContent-Type: text/xml; charset=utf-8\r\nCache-Control: no-cache\r\n\r\n"));
}

static int
http_get_display_power ()
{
    http_send(FS("HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\n"));
    http_send(get_numvar (DISPLAY_POWER_NUM_VAR) ? "on\n" : "off\n");
    http_flush();

    return 0;
}

static int
http_get_ambilight_power ()
{
    http_send(FS("HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\n"));
    http_send(get_numvar (DISPLAY_AMBILIGHT_POWER_NUM_VAR) ? "on\n" : "off\n");
    http_flush();

    return 0;
}

static int
http_api_display_power ()
{
    http_send(FS("HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nCache-Control: no-cache\r\n\r\n"));
    http_send(get_numvar (DISPLAY_POWER_NUM_VAR) ? "on\n" : "off\n");
    http_flush();

    return 0;
}

static int
http_api_ambilight_power ()
{
    http_send(FS("HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nCache-Control: no-cache\r\n\r\n"));
    http_send(get_numvar (DISPLAY_AMBILIGHT_POWER_NUM_VAR) ? "on\n" : "off\n");
    http_flush();

    return 0;
}

static int
http_api_power_status ()
{
    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"display_power\":\""));
    http_send(get_numvar (DISPLAY_POWER_NUM_VAR) ? "on" : "off");
    http_send (FS("\",\"ambilight_power\":\""));
    http_send(get_numvar (DISPLAY_AMBILIGHT_POWER_NUM_VAR) ? "on" : "off");
    http_send (FS("\"}"));
    http_flush();

    return 0;
}

static int
http_api_display_power_set ()
{
    char * value = http_get_param ("value");

    if (! strcmp (value, "on"))
    {
        set_numvar (DISPLAY_POWER_NUM_VAR, 1);
    }
    else if (! strcmp (value, "off"))
    {
        set_numvar (DISPLAY_POWER_NUM_VAR, 0);
    }

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"display_power\":\""));
    http_send (get_numvar (DISPLAY_POWER_NUM_VAR) ? "on" : "off");
    http_send (FS("\"}"));
    http_flush ();

    return 0;
}

static int
http_api_ambilight_power_set ()
{
    char * value = http_get_param ("value");

    if (! strcmp (value, "on"))
    {
        set_numvar (DISPLAY_AMBILIGHT_POWER_NUM_VAR, 1);
    }
    else if (! strcmp (value, "off"))
    {
        set_numvar (DISPLAY_AMBILIGHT_POWER_NUM_VAR, 0);
    }

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"ambilight_power\":\""));
    http_send (get_numvar (DISPLAY_AMBILIGHT_POWER_NUM_VAR) ? "on" : "off");
    http_send (FS("\"}"));
    http_flush ();

    return 0;
}

static int
http_api_display_brightness_set ()
{
    int brightness;

    /* Ein leeres Feld darf nicht still als 0 gelten - das schaltet das Display
     * dunkel, obwohl gar keine Helligkeit angegeben wurde (L29).
     */
    if (! http_get_int_param ("value", &brightness))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or not numeric");
        return 0;
    }

    /* Abweisen statt klemmen: {"ok":true} heisst, dass der GESENDETE Wert gilt
     * (Parametervertrag Runde 1, L186). Die Grenze selbst bleibt stehen - ohne sie
     * verkuerzt set_numvar einen zu grossen Wert still auf 16 Bit (L198).
     */
    if (brightness < 0 || brightness > 15)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "value out of range (0..15)");
        return 0;
    }

    set_numvar (DISPLAY_BRIGHTNESS_NUM_VAR, brightness);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"display_brightness\":"));
    char buf[8];
    sprintf (buf, "%d", brightness);
    http_send (buf);
    http_send (FS("}"));
    http_flush ();

    return 0;
}

static int
http_api_auto_brightness_set ()
{
    char * value = http_get_param ("value");

    if (! strcmp (value, "on"))
    {
        set_numvar (DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE_NUM_VAR, 1);
    }
    else if (! strcmp (value, "off"))
    {
        set_numvar (DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE_NUM_VAR, 0);
    }

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"auto_brightness\":\""));
    http_send (get_numvar (DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE_NUM_VAR) ? "on" : "off");
    http_send (FS("\"}"));
    http_flush ();

    return 0;
}

static int
http_api_display_mode_set ()
{
    int mode;

    /* Leer heisst nicht 0 - sonst springt die Anzeige auf den ersten Modus,
     * obwohl niemand einen Modus gewaehlt hat (L29).
     */
    if (! http_get_int_param ("value", &mode))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or not numeric");
        return 0;
    }

    /* Abweisen statt klemmen (L186). Die Obergrenze ist ein Laufzeitwert und laesst
     * sich im Text nicht als Literal schreiben - snprintf in einen Stackpuffer, kein
     * String (L175).
     */
    if (mode < 0 || mode >= (int) display_modes_count)
    {
        char detail[44];

        snprintf (detail, sizeof (detail), "value out of range (0..%d)",
                  display_modes_count ? (int) display_modes_count - 1 : 0);
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, detail);
        return 0;
    }

    set_numvar (DISPLAY_MODE_NUM_VAR, mode);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"display_mode\":"));
    char buf[8];
    sprintf (buf, "%d", mode);
    http_send (buf);
    http_send (FS("}"));
    http_flush ();

    return 0;
}

static int
http_api_display_it_is_set ()
{
    uint_fast8_t flags = get_numvar (DISPLAY_FLAGS_NUM_VAR);

    if (http_get_on_off_value ("value", flags & DISPLAY_FLAGS_PERMANENT_IT_IS))
    {
        flags |= DISPLAY_FLAGS_PERMANENT_IT_IS;
    }
    else
    {
        flags &= ~DISPLAY_FLAGS_PERMANENT_IT_IS;
    }

    set_numvar (DISPLAY_FLAGS_NUM_VAR, flags);
    http_json_ok ();

    return 0;
}

static int
http_api_ticker_set ()
{
    char * value = http_get_param ("value");

    /* Bewusste Ausnahme von L48: Der leere Tickertext ist der Auslieferungszustand und
     * zugleich der einzige Weg, den Ticker wieder abzuschalten - es gibt keinen
     * getrennten Schalter. Eine Ablehnung des leeren Werts liesse ihn nie mehr loeschen.
     */
    if (! http_check_strvar_len ("value", value, MAX_TICKER_TEXT_LEN))
    {
        return 0;
    }

    set_strvar (TICKER_TEXT_STR_VAR, value);

    http_json_ok ();

    return 0;
}

static int
http_api_date_ticker_format_set ()
{
    char * value;

    /* Leer heisst hier nicht "kein Datum", sondern ein Formatstring ohne Platzhalter:
     * Der Ticker laeuft durch und zeigt nichts. Ausgeloest wird die Datumsanzeige ueber
     * einen Overlay-Eintrag bzw. eine RPC, nicht ueber dieses Feld - es ist also kein
     * Abschalter, und die Vorgabe "D.M.Y" waere unwiederbringlich weg (L48).
     */
    if (! http_get_string_param ("value", &value))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or empty");
        return 0;
    }

    if (! http_check_strvar_len ("value", value, MAX_DATE_TICKER_FORMAT_LEN))
    {
        return 0;
    }

    set_strvar (DATE_TICKER_FORMAT_VAR, value);

    http_json_ok ();

    return 0;
}

static int
http_api_ticker_deceleration_set ()
{
    int deceleration;

    /* Leer heisst nicht 0 - eine 0 laesst den Ticker mit voller Geschwindigkeit
     * laufen und ist unlesbar (L29).
     */
    if (! http_get_int_param ("value", &deceleration))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or not numeric");
        return 0;
    }

    /* Abweisen statt klemmen (L186). Die Grenze bleibt stehen: Der Setter schickt den
     * Wert mit "%02x" auf die Bruecke, und %02x ist eine Mindest-, keine Hoechstbreite -
     * ueber 255 entstehen drei Hexziffern und verschieben das ganze Kommando (L66, L198).
     */
    if (deceleration < 0 || deceleration > 255)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "value out of range (0..255)");
        return 0;
    }

    set_numvar (TICKER_DECELRATION_NUM_VAR, deceleration);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"ticker_deceleration\":"));
    char buf[8];
    sprintf (buf, "%d", deceleration);
    http_send (buf);
    http_send (FS("}"));
    http_flush ();

    return 0;
}

static int
http_api_test_display ()
{
    rpc (TEST_DISPLAY_RPC_VAR);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();

    return 0;
}

static int
http_api_weather_appid_set ()
{
    char * value;

    /* Ein leerer Schluessel nimmt der Uhr das Wetter vollstaendig und wurde bisher mit
     * Erfolg quittiert (L48). Zum Abschalten des Wetters gibt es den Weg ueber die
     * Overlays; der Schluessel ist keine Ja/Nein-Einstellung.
     */
    if (! http_get_string_param ("value", &value))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or empty");
        return 0;
    }

    if (! http_check_strvar_len ("value", value, MAX_WEATHER_APPID_LEN))
    {
        return 0;
    }

    set_strvar (WEATHER_APPID_STR_VAR, value);

    http_json_ok ();

    return 0;
}

static int
http_api_weather_city_set ()
{
    char *      value = http_get_param ("value");
    STR_VAR *   lon_var = get_strvar (WEATHER_LON_STR_VAR);
    STR_VAR *   lat_var = get_strvar (WEATHER_LAT_STR_VAR);

    /* Ort und Koordinaten sind Alternativen - der STM32 nimmt die Koordinaten, sobald
     * beide gefuellt sind, sonst den Ort. Ein pauschales Verbot des leeren Werts (L48)
     * wuerde den einmal gewaehlten Weg fuer immer festschreiben. Leer ist deshalb
     * zulaessig, solange die andere Ortsangabe bestehen bleibt - nur der Fall "beides
     * leer" nimmt der Uhr still das Wetter.
     */
    if (! http_check_strvar_len ("value", value, MAX_WEATHER_CITY_LEN))
    {
        return 0;
    }

    if (! *value)
    {
        if (! lon_var || ! *(lon_var->str) || ! lat_var || ! *(lat_var->str))
        {
            http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or empty and no coordinates configured");
            return 0;
        }

        value = (char *) "";
    }

    set_strvar (WEATHER_CITY_STR_VAR, value);

    http_json_ok ();

    return 0;
}

static int
http_api_weather_coordinates_set ()
{
    char *      lon = http_get_param ("lon");
    char *      lat = http_get_param ("lat");
    STR_VAR *   city_var = get_strvar (WEATHER_CITY_STR_VAR);

    if (! http_check_strvar_len ("lon", lon, MAX_WEATHER_LON_LEN) ||
        ! http_check_strvar_len ("lat", lat, MAX_WEATHER_LAT_LEN))
    {
        return 0;
    }

    /* Nur eine der beiden Koordinaten zu setzen ergibt nie eine Abfrage: Der STM32
     * verlangt beide und faellt sonst auf den Ort zurueck - der halbe Wert bliebe als
     * stiller Ballast stehen (L48).
     */
    if ((*lon && ! *lat) || (! *lon && *lat))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "lon and lat required together");
        return 0;
    }

    /* Leere Koordinaten sind der einzige Weg zurueck zur Ortsabfrage, denn der STM32
     * bevorzugt die Koordinaten, sobald beide gefuellt sind. Sie sind deshalb erlaubt,
     * solange ein Ort eingetragen bleibt (L48).
     */
    if (! *lon && ! *lat && (! city_var || ! *(city_var->str)))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "lon and lat missing or empty and no city configured");
        return 0;
    }

    strsubst (lon, ',', '.');
    strsubst (lat, ',', '.');

    set_strvar (WEATHER_LON_STR_VAR, lon);
    set_strvar (WEATHER_LAT_STR_VAR, lat);

    http_json_ok ();

    return 0;
}

/* Der Abruf selbst laeuft asynchron: Der ESP stoesst nur eine RPC am STM32 an, der
 * fragt Sekunden spaeter ueber die UART die Wetterdaten an und zeigt sie an. Ob
 * OpenWeatherMap die Anfrage beantwortet, ist zum Zeitpunkt der HTTP-Antwort also nicht
 * feststellbar - ein ehrliches {"ok":true} gibt es hier nicht (L54).
 * Feststellbar sind die Voraussetzungen: ohne Schluessel, ohne Ortsangabe oder ohne
 * Verbindung ins Netz kann der Abruf gar nicht gelingen. Genau diese Faelle hat die
 * Oberflaeche bisher als "abgerufen" gemeldet.
 */
static int
http_api_weather_request (RPC_VARIABLE rpc_var)
{
    STR_VAR *   appid_var = get_strvar (WEATHER_APPID_STR_VAR);
    STR_VAR *   city_var = get_strvar (WEATHER_CITY_STR_VAR);
    STR_VAR *   lon_var = get_strvar (WEATHER_LON_STR_VAR);
    STR_VAR *   lat_var = get_strvar (WEATHER_LAT_STR_VAR);
    uint_fast8_t has_city = (city_var && *(city_var->str)) ? 1 : 0;
    uint_fast8_t has_coordinates = (lon_var && *(lon_var->str) && lat_var && *(lat_var->str)) ? 1 : 0;

    if (! appid_var || ! *(appid_var->str))
    {
        http_json_error (HTTP_API_ERROR_NOT_CONFIGURED, "weather appid not configured");
        return 0;
    }

    if (! has_city && ! has_coordinates)
    {
        http_json_error (HTTP_API_ERROR_NOT_CONFIGURED, "neither city nor coordinates configured");
        return 0;
    }

    if (wifi_ap_mode)                                                               // eigener Accesspoint, kein Weg ins Internet
    {
        http_json_error (HTTP_API_ERROR_NOT_CONFIGURED, "no internet connection in ap mode");
        return 0;
    }

    rpc (rpc_var);

    http_json_ok ();

    return 0;
}

static int
http_api_weather_get_now ()
{
    return http_api_weather_request (GET_WEATHER_RPC_VAR);
}

static int
http_api_weather_get_forecast ()
{
    return http_api_weather_request (GET_WEATHER_FC_RPC_VAR);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * /api/network_scan - Liste der sichtbaren WLAN-Netze (BEFUNDE.md L174)
 *
 * Hier stand der WLAN-Scan MITTEN IN DER LAUFENDEN ANTWORT: Kopfzeilen und die ersten
 * Felder waren schon hinaus, erst danach kam WiFi.scanNetworks (). Das ist der
 * unguenstigste denkbare Zeitpunkt - lwIP haelt in dem Moment die Sendepuffer der
 * begonnenen Antwort, waehrend der Scan seine Ergebnisliste am Haufen anlegt. Am
 * 04.10.2026 endete das in einem OOM-Abbruch: 6'016 Byte frei, groesster Block 5'752,
 * fehlgeschlagene Anforderung 960.
 *
 * Drei Aenderungen, alle ohne zusaetzlichen Speicherbedarf:
 *
 * 1. Der Scan laeuft VOR dem ersten http_send (). Dann steht ihm der volle Haufen zur
 *    Verfuegung, und scheitert er trotzdem, ist noch keine Antwort angefangen.
 * 2. KEIN String mehr je Netz. Bisher entstanden zwei: WiFi.SSID (idx) liefert einen,
 *    sanitize_xml_string () baut daraus zeichenweise einen zweiten. Bei zwanzig Netzen
 *    summiert sich das mitten in der Antwort. Stattdessen WiFi.getScanInfoByIndex (),
 *    das einen Zeiger auf den vorhandenen Eintrag liefert, und http_escape_text () in
 *    einen Puffer auf dem Stack. Je Netz genau ein http_send ().
 * 3. OBERGRENZE. Ohne sie haengt die Groesse der Antwort davon ab, wie viele Netze
 *    zufaellig in der Luft sind - keine Eigenschaft, die man steuern kann. Die Grenze
 *    begrenzt die AUSGABE; was der Scan selbst belegt, bestimmt weiterhin das SDK.
 *    Deshalb wird die Ergebnisliste am Ende ausdruecklich mit scanDelete () freigegeben,
 *    statt bis zum naechsten Scan liegen zu bleiben.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define HTTP_MAX_SCAN_NETWORKS      24                          // Obergrenze fuer die ausgegebene Netzliste

static int
http_api_network_scan ()
{
    char    buf[232];                                           // 32 Zeichen SSID, je bis 6 Byte maskiert, plus Rahmen
    char    ssid[33];                                           // bss_info.ssid ist 32 Byte und nicht zwingend nullterminiert
    int     networks;
    int     idx;
    int     first = 1;

    networks = WiFi.scanNetworks ();                            // (1) vor der ersten Ausgabe, bei vollem Haufen

    if (networks < 0)                                           // WIFI_SCAN_FAILED / WIFI_SCAN_RUNNING
    {
        networks = 0;
    }

    if (networks > HTTP_MAX_SCAN_NETWORKS)
    {
        networks = HTTP_MAX_SCAN_NETWORKS;                      // (3)
    }

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ssid\":\""));
    http_escape_text (wifi_ssid, (unsigned int) strlen (wifi_ssid), buf, sizeof (buf), HTTP_ESCAPE_SCAN, (unsigned int *) 0);
    http_send (buf);
    http_send (FS("\",\"ip\":\""));
    http_escape_text (wifi_ip_address, (unsigned int) strlen (wifi_ip_address), buf, sizeof (buf), HTTP_ESCAPE_SCAN, (unsigned int *) 0);
    http_send (buf);
    http_send (FS("\",\"mode\":\""));
    http_send (wifi_ap_mode ? "ap" : "client");
    http_send (FS("\",\"networks\":["));

    for (idx = 0; idx < networks; idx++)
    {
        const bss_info *    info = WiFi.getScanInfoByIndex (idx);
        size_t              n;

        if (! info)
        {
            continue;                                           // Liste wurde zwischendurch verworfen
        }

        memcpy (ssid, info->ssid, sizeof (info->ssid));
        ssid[sizeof (info->ssid)] = '\0';

        /* Derselbe Name kommt je Accesspoint und Kanal mehrfach. Ohne Feldstaerke kann
         * die PWA beim Entfernen der Dubletten nur den ersten Treffer behalten, nicht
         * den besten (L41). Sie nimmt beide Formen entgegen: blosser Name oder Objekt.
         */
        n = 0;

        if (! first)
        {
            buf[n++] = ',';
        }

        first = 0;

        memcpy (buf + n, "{\"ssid\":\"", 9);
        n += 9;
        n += http_escape_text (ssid, (unsigned int) strlen (ssid), buf + n, sizeof (buf) - n, HTTP_ESCAPE_SCAN, (unsigned int *) 0);
        snprintf (buf + n, sizeof (buf) - n, "\",\"rssi\":%d}", (int) info->rssi);

        http_send (buf);                                        // (2) genau ein Aufruf je Netz, kein String
    }

    http_send (FS("]}"));
    http_flush ();

    WiFi.scanDelete ();                                         // Ergebnisliste sofort freigeben, nicht erst beim naechsten Scan

    return 0;
}

static int
http_api_network_client_set ()
{
    char * ssid = http_get_param ("ssid");
    char * key  = http_get_param ("key");

    /* C38/L323: abweisen statt still kuerzen - VOR wifi_connect () und vor jedem Schreibzugriff.
     * Die Grenze ist das, was toCharArray () unten tatsaechlich uebernimmt: Puffergroesse
     * EEPROM_*_LEN, also hoechstens EEPROM_*_LEN - 1 Byte (31 bzw. 63). Bisher verband sich das
     * Geraet mit dem vollen Wert und speicherte den gekuerzten.
     */
    if (! http_check_strvar_len ("ssid", ssid, EEPROM_SSID_LEN - 1) || ! http_check_strvar_len ("key", key, EEPROM_SSID_KEY_LEN - 1))
    {
        return 0;
    }

    wifi_connect (ssid, key, true);

    String pssid = ssid;
    String pkey = key;

    if (! pssid.equals (eeprom_ssid))
    {
        pssid.toCharArray (eeprom_ssid, EEPROM_SSID_LEN);
        eeprom_save_ssid ();
    }

    if (! pkey.equals (eeprom_ssidkey))
    {
        pkey.toCharArray (eeprom_ssidkey, EEPROM_SSID_KEY_LEN);
        eeprom_save_ssidkey ();
    }

    eeprom_flags &= ~EEPROM_FLAG_BOOT_AS_AP;
    eeprom_save_flags ();
    eeprom_commit ();

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();

    return 0;
}

static int
http_api_network_ap_set ()
{
    char * ssid = http_get_param ("ssid");
    char * key  = http_get_param ("key");

    /* Bei zu kurzem Schluessel passierte bisher gar nichts, die Antwort lautete aber
     * {"ok":true} und die Oberflaeche meldete "Zugangspunkt gestartet" (L30).
     */
    if (strlen (key) < 10)
    {
        http_json_error (HTTP_API_ERROR_TOO_SHORT, "ap key too short (min. 10 characters)");
        return 0;
    }

    /* C38 / Review F.6 A4: dieselbe Regel wie http_api_network_client_set () - abweisen statt
     * still kuerzen, vor jedem Schreibzugriff und vor wifi_ap ().
     */
    if (! http_check_strvar_len ("ssid", ssid, EEPROM_AP_SSID_LEN - 1) || ! http_check_strvar_len ("key", key, EEPROM_AP_SSID_KEY_LEN - 1))
    {
        return 0;
    }

    String ap_ssid = ssid;
    String ap_key  = key;

    if (! ap_ssid.equals (eeprom_ap_ssid))
    {
        ap_ssid.toCharArray (eeprom_ap_ssid, EEPROM_AP_SSID_LEN);
        eeprom_save_ap_ssid ();
    }

    if (! ap_key.equals (eeprom_ap_ssidkey))
    {
        ap_key.toCharArray (eeprom_ap_ssidkey, EEPROM_AP_SSID_KEY_LEN);
        eeprom_save_ap_ssidkey ();
    }

    eeprom_flags |= EEPROM_FLAG_BOOT_AS_AP;
    eeprom_save_flags ();
    eeprom_commit ();

    wifi_ap (ssid, key);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();

    return 0;
}

static int
http_api_eeprom_settings ()
{
    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"ssid\":\""));
    http_send_json_escaped (eeprom_ssid);
    http_send (FS("\",\"key\":\""));
    http_send_json_escaped (eeprom_ssidkey);
    http_send (FS("\",\"ap_ssid\":\""));
    http_send_json_escaped (eeprom_ap_ssid);
    http_send (FS("\",\"ap_key\":\""));
    http_send_json_escaped (eeprom_ap_ssidkey);
    http_send (FS("\",\"flags\":"));
    http_send (eeprom_flags ? "1" : "0");
    http_send (FS(",\"boot_as_ap\":"));
    http_send ((eeprom_flags & EEPROM_FLAG_BOOT_AS_AP) ? "true" : "false");
    http_send (FS("}"));
    http_flush ();

    return 0;
}

static int
http_api_eeprom_settings_set ()
{
    char * ssid = http_get_param ("ssid");
    char * key = http_get_param ("key");
    char * ap_ssid = http_get_param ("ap_ssid");
    char * ap_key = http_get_param ("ap_key");
    char * boot_as_ap = http_get_param ("boot_as_ap");

    String pssid = ssid;
    String pkey = key;
    String pap_ssid = ap_ssid;
    String pap_key = ap_key;

    /* Ein LEERES Feld heisst "nicht aendern", nicht "loeschen".
     *
     * Der Backup-Import der PWA sendet diese fuenf Felder bedingungslos. Lief beim
     * Export der Abruf von /api/eeprom_settings in einen Timeout, stehen im
     * Sicherungsdokument vier leere Zeichenketten -- und der Import schrieb sie
     * zurueck. Danach hatte das Geraet keine WLAN-Zugangsdaten mehr, und weil der
     * AP-Schalter im selben Zug geloescht wurde, auch keinen Accesspoint. wifi.cpp
     * startet den Webserver nur im Erfolgszweig: Das Geraet war ueber Netzwerk nicht
     * mehr erreichbar, Rueckweg nur ueber die serielle Schnittstelle.
     *
     * Wer eine Zugangskennung wirklich entfernen will, setzt eine neue -- ein
     * versehentlich leeres Feld darf eine funktionierende Konfiguration nie loeschen.
     */

    /* C38/L323: zu lange Werte abweisen statt still kuerzen, ALLE VIER vor dem ersten
     * Schreibzugriff - sonst stuende nach einer Abweisung ein halber Satz im EEPROM. Grenze wie
     * in http_api_network_client_set (): EEPROM_*_LEN - 1, mehr nimmt toCharArray () nicht.
     */
    if (! http_check_strvar_len ("ssid", ssid, EEPROM_SSID_LEN - 1)
     || ! http_check_strvar_len ("key", key, EEPROM_SSID_KEY_LEN - 1)
     || ! http_check_strvar_len ("ap_ssid", ap_ssid, EEPROM_AP_SSID_LEN - 1)
     || ! http_check_strvar_len ("ap_key", ap_key, EEPROM_AP_SSID_KEY_LEN - 1))
    {
        return 0;
    }

    if (pssid.length () > 0 && ! pssid.equals (eeprom_ssid))
    {
        pssid.toCharArray (eeprom_ssid, EEPROM_SSID_LEN);
        eeprom_save_ssid ();
    }

    if (pkey.length () > 0 && ! pkey.equals (eeprom_ssidkey))
    {
        pkey.toCharArray (eeprom_ssidkey, EEPROM_SSID_KEY_LEN);
        eeprom_save_ssidkey ();
    }

    if (pap_ssid.length () > 0 && ! pap_ssid.equals (eeprom_ap_ssid))
    {
        pap_ssid.toCharArray (eeprom_ap_ssid, EEPROM_AP_SSID_LEN);
        eeprom_save_ap_ssid ();
    }

    if (pap_key.length () > 0 && ! pap_key.equals (eeprom_ap_ssidkey))
    {
        pap_key.toCharArray (eeprom_ap_ssidkey, EEPROM_AP_SSID_KEY_LEN);
        eeprom_save_ap_ssidkey ();
    }

    if (! strcmp (boot_as_ap, "on"))
    {
        eeprom_flags |= EEPROM_FLAG_BOOT_AS_AP;
    }
    else if (eeprom_ssid[0])
    {
        /* Den Accesspoint nur abschalten, wenn danach noch ein WLAN bleibt. Sonst
         * faellt der letzte Weg ins Geraet weg.
         */
        eeprom_flags &= ~EEPROM_FLAG_BOOT_AS_AP;
    }

    eeprom_save_flags ();
    eeprom_commit ();

    http_json_ok ();

    return 0;
}

static int
http_api_network_timeserver_set ()
{
    char * value;

    /* Ein leeres Zeitserverfeld nimmt der Uhr die Zeitquelle - und wurde mit gruenem
     * "Gespeichert" quittiert (L48). Einen Nutzen hat der leere Wert nicht: Ohne
     * Zeitserver laeuft die Uhr nur noch auf der RTC.
     */
    if (! http_get_string_param ("value", &value))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or empty");
        return 0;
    }

    if (! http_check_strvar_len ("value", value, MAX_TIMESERVER_NAME_LEN))
    {
        return 0;
    }

    set_strvar (TIMESERVER_STR_VAR, value);

    http_json_ok ();

    return 0;
}

static int
http_api_network_timezone_set ()
{
    int tz;
    uint_fast16_t utz = 0;

    /* Die Grenzen -12..14 standen bisher nur in der PWA. Legacy-Oberflaeche, direkter
     * API-Aufruf und Backup-Import gehen daran vorbei: Ab 256 kippt das Vorzeichenbit
     * 0x100, ab 512 kollidiert der Wert mit dem Sommerzeitbit 0x200 (L28).
     */
    if (! http_get_int_param ("value", &tz))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or not numeric");
        return 0;
    }

    if (tz < -12 || tz > 14)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "timezone out of range (-12..14)");
        return 0;
    }

    if (tz < 0)
    {
        utz = -tz;
        utz |= 0x100;
    }
    else
    {
        utz = tz;
    }

    if (get_numvar (TIMEZONE_NUM_VAR) & 0x200)
    {
        utz |= 0x200;
    }

    set_numvar (TIMEZONE_NUM_VAR, utz);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();

    return 0;
}

static int
http_api_network_summertime_set ()
{
    uint_fast16_t utz = get_numvar (TIMEZONE_NUM_VAR);

    /* Einziger on/off-Setter mit bedingungslosem else: Ein fehlender Parameter hat die
     * Sommerzeit abgeschaltet und Erfolg gemeldet - die Uhr ging danach eine Stunde
     * falsch (L47). Alle uebrigen Setter lassen den aktuellen Wert stehen.
     */
    if (http_get_on_off_value ("value", (utz & 0x200) ? 1 : 0))
    {
        utz |= 0x200;
    }
    else
    {
        utz &= ~0x200;
    }

    set_numvar (TIMEZONE_NUM_VAR, utz);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();

    return 0;
}

static int
http_api_network_get_time ()
{
    rpc (GET_NET_TIME_RPC_VAR);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();

    return 0;
}

static int
http_api_network_wps ()
{
    wifi_wps ();

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();

    return 0;
}

static int
http_api_update_host_set ()
{
    char * value;

    /* Leerer Host heisst: kein Update mehr moeglich, und der Weg zurueck fuehrt nur
     * ueber die Legacy-Oberflaeche. Dieselbe Luecke wie L42, nur von vorne (L48).
     */
    if (! http_get_string_param ("value", &value))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or empty");
        return 0;
    }

    if (! http_check_strvar_len ("value", value, MAX_UPDATE_HOST_LEN))
    {
        return 0;
    }

    set_strvar (UPDATE_HOST_VAR, value);

    http_json_ok ();

    return 0;
}

static int
http_api_update_path_set ()
{
    char * value;

    /* Wie beim Host: ein leerer Pfad macht die Update-Quelle unbrauchbar und wurde als
     * Erfolg gemeldet (L48, dieselbe Luecke wie L42).
     */
    if (! http_get_string_param ("value", &value))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or empty");
        return 0;
    }

    if (! http_check_strvar_len ("value", value, MAX_UPDATE_PATH_LEN))
    {
        return 0;
    }

    set_strvar (UPDATE_PATH_VAR, value);

    http_json_ok ();

    return 0;
}

static int
http_api_update_download_assets ()
{
    const char * fname_icon = (const char *) 0;
    const char * fname_weather = (const char *) 0;
    int download_rtc = 2;
    STR_VAR * sv;
    char * update_host;
    char * update_path;

    if (hardware_configuration != 0xFFFF)
    {
        switch (hardware_configuration & HW_WC_MASK)
        {
            case HW_WC_24H:
                fname_icon = "wc24h-icon.txt";
                fname_weather = "wc24h-weather.txt";
                break;
            case HW_WC_12H:
                fname_icon = "wc12h-icon.txt";
                fname_weather = "wc12h-weather.txt";
                break;
            case HW_UCLOCK:
                fname_icon = "uc-icon.txt";
                fname_weather = "uc-weather.txt";
                break;
        }
    }

    sv = get_strvar (UPDATE_HOST_VAR);
    update_host = sv->str;
    sv = get_strvar (UPDATE_PATH_VAR);
    update_path = sv->str;

    LittleFS.begin ();

    if (fname_icon)
    {
        download_rtc = download_file (update_host, update_path, fname_icon);
    }

    if (fname_weather)
    {
        download_rtc = download_file (update_host, update_path, fname_weather);
    }

    LittleFS.end ();

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":"));
    http_send (download_rtc == 1 ? "true" : "false");
    http_send (FS("}"));
    http_flush ();

    return 0;
}

static int
http_api_update_download_table ()
{
    STR_VAR *       sv;
    char *          update_host;
    char *          update_path;
    char *          filename = http_get_param ("filename");
    int             download_rtc = 0;

    if (! http_table_download_filename_matches (filename))
    {
        http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
        http_send (FS("{\"ok\":false,\"error\":\"invalid_filename\"}"));
        http_flush ();
        return 0;
    }

    sv = get_strvar (UPDATE_HOST_VAR);
    update_host = sv->str;

    if (! update_host[0])
    {
        update_host = (char *) DEFAULT_UPDATE_HOST;
    }

    sv = get_strvar (UPDATE_PATH_VAR);
    update_path = sv->str;

    if (! update_path[0])
    {
        update_path = (char *) DEFAULT_UPDATE_PATH;
    }

    LittleFS.begin ();
    download_rtc = download_file (update_host, update_path, filename);
    LittleFS.end ();

    if (download_rtc == 1)
    {
        tables_init ();
    }

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":"));
    http_send (download_rtc == 1 ? "true" : "false");

    if (download_rtc != 1)
    {
        http_send (FS(",\"error\":\"download_failed\""));
    }

    http_send (FS("}"));
    http_flush ();

    return 0;
}

static int
http_api_maintenance_format_fs ()
{
    if (http_request_is_embedded_subresource ())
    {
        http_deny_embedded_subresource ("maintenance_format_fs");
        return 0;
    }

    LittleFS.begin ();
    LittleFS.format ();
    LittleFS.end ();

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();

    return 0;
}

static int
http_api_maintenance_reset_stm32 ()
{
    /* Bewusst auch hier, obwohl ein STM-Reset nichts loescht: Daran laesst sich die
     * Schutzlogik pruefen, ohne Daten zu riskieren -- die beiden anderen Endpunkte
     * kann man zum Testen nicht aufrufen.
     */
    if (http_request_is_embedded_subresource ())
    {
        http_deny_embedded_subresource ("maintenance_reset_stm32");
        return 0;
    }

    update_progress_state ("reset", "STM32 wird zurückgesetzt.");
    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();
    delay (200);
    stm32_reset ();
    update_progress_complete ("STM32 wurde zurückgesetzt.");

    return 0;
}

static int
http_api_maintenance_reset_eeprom ()
{
    if (http_request_is_embedded_subresource ())
    {
        http_deny_embedded_subresource ("maintenance_reset_eeprom");
        return 0;
    }

    rpc (RESET_EEPROM_RPC_VAR);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();

    return 0;
}

static int
http_api_datetime_set ()
{
    TM  tm;
    int year;
    int month;
    int day;
    int hour;
    int minute;

    /* Fehlende Felder nicht stillschweigend auf 0 bzw. den Mindestwert ziehen -
     * sonst stellt ein unvollstaendiger Aufruf die Uhr auf den 01.01.2000 (L29).
     */
    if (! http_get_int_param ("year", &year) ||
        ! http_get_int_param ("month", &month) ||
        ! http_get_int_param ("day", &day) ||
        ! http_get_int_param ("hour", &hour) ||
        ! http_get_int_param ("minute", &minute))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "year, month, day, hour and minute required");
        return 0;
    }

    /* Bisher wurde jedes Feld nur einzeln begrenzt. Damit kam der 31. Februar
     * widerspruchslos durch, und zu grosse Werte wurden stumm zurechtgebogen
     * statt abgewiesen (L32).
     */
    if (year < 2000 || year > 2999 ||
        month < 1 || month > 12 ||
        day < 1 || day > (int) http_days_in_month (year, month) ||
        hour < 0 || hour > 23 ||
        minute < 0 || minute > 59)
    {
        http_json_error (HTTP_API_ERROR_INVALID_DATE, "invalid date or time");
        return 0;
    }

    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = 0;
    tm.tm_wday = dayofweek (day, month, year);

    set_tm_var (CURRENT_TM_VAR, &tm);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();

    return 0;
}

static int
http_api_learn_ir ()
{
    rpc (LEARN_IR_RPC_VAR);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();

    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * IR-Codes: Abzug anstossen
 *
 * Die Reihenfolge ist wesentlich: erst den Puffer leeren, dann den RPC ausloesen. Umgekehrt liefen die
 * ersten I-Kommandos des STM in einen Puffer, den ir_codes_begin_request() gleich darauf wieder
 * verwirft -- sie fehlten in der Maske, und complete wuerde nie true.
 *
 * Ein STM-Kommando pro Aufruf: der RPC selbst. Die 20 Antwortkommandos sendet der STM getaktet, je
 * eines pro Hauptloop-Durchlauf (specs/f1-ir-backup/design.md, "Abweichung von der Analyse"), also
 * kein Burst auf dem 256-Byte-RX-Ring und keine Schleife ohne watchdog_reload().
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static int
http_api_ir_codes_request ()
{
    char    buf[8];

    ir_codes_begin_request ();

    /* Der Rueckgabewert von rpc() wird ausgewertet statt verworfen: Bliebe requested auf 1, ohne dass
     * je ein I-Kommando unterwegs ist, pollte die PWA sechs Sekunden gegen einen Puffer, den niemand
     * fuellt -- und ein Nachzuegler eines frueheren Abzugs saehe dann aus wie ein frischer Wert.
     */
    if (! rpc (GET_IR_CODES_RPC_VAR))
    {
        ir_codes_invalidate ();
        http_json_error (HTTP_API_ERROR_NOT_CONFIGURED, "ir codes rpc not available");
        return 0;
    }

    sprintf (buf, "%d", (int) MAX_IR_CODES);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"expected\":"));
    http_send (buf);
    http_send (FS("}"));
    http_flush ();

    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * IR-Codes: Abzug lesen
 *
 * Rein lesend. Kein STM-Kommando, kein Anstoss -- wer wissen will, was im Puffer liegt, loest damit
 * keinen neuen Abzug aus.
 *
 * requested wird ZUERST ausgewertet, nicht erst complete. Ein verspaetetes I-Kommando kann nach einer
 * Invalidierung noch Maskenbits setzen. Haengt die Antwort allein an der Maske, saehe die PWA nach
 * einem ESP-Neustart mitten im Abzug einen teilgefuellten Puffer ohne jede Warnung und schriebe ihn
 * als gueltige Sicherung weg. Ist requested 0, gilt der Puffer deshalb als nicht vorhanden:
 * received 0, complete false, alle Indizes in missing[], codes[] leer.
 *
 * codes[] traegt nur eingetroffene Indizes, jeder mit seinem idx. Fehlende stehen in missing[] --
 * das ist eindeutig und spart Bytes gegenueber 20 Eintraegen mit Fuellwerten. Vollausbau rund 1,1 kB.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static int
http_api_ir_codes_get ()
{
    char            buf[16];
    uint_fast8_t    requested;
    uint_fast8_t    received;
    uint_fast8_t    complete;
    uint32_t        mask;
    uint_fast8_t    idx;
    uint_fast8_t    first;

    requested   = ir_codes_are_requested () ? 1 : 0;
    mask        = requested ? ir_codes_mask () : 0;
    received    = requested ? ir_codes_count () : 0;
    complete    = (requested && ir_codes_is_complete ()) ? 1 : 0;

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"requested\":"));
    http_send (requested ? "true" : "false");

    sprintf (buf, "%d", (int) MAX_IR_CODES);
    http_send (FS(",\"expected\":"));
    http_send (buf);

    sprintf (buf, "%d", (int) received);
    http_send (FS(",\"received\":"));
    http_send (buf);

    http_send (FS(",\"complete\":"));
    http_send (complete ? "true" : "false");

    http_send (FS(",\"missing\":["));
    first = 1;

    for (idx = 0; idx < MAX_IR_CODES; idx++)
    {
        if (! (mask & (((uint32_t) 1) << idx)))
        {
            if (! first)
            {
                http_send (FS(","));
            }

            sprintf (buf, "%d", (int) idx);
            http_send (buf);
            first = 0;
        }
    }

    http_send (FS("],\"codes\":["));
    first = 1;

    if (requested)
    {
        for (idx = 0; idx < MAX_IR_CODES; idx++)
        {
            /* Gegen dieselbe Momentaufnahme wie missing[] oben: Beide Listen beschreiben damit
             * nachweislich denselben Abzug und koennen nicht gegeneinander laufen.
             */
            IR_CODE *   ir = (mask & (((uint32_t) 1) << idx)) ? get_ir_code (idx) : (IR_CODE *) 0;

            if (ir)
            {
                if (! first)
                {
                    http_send (FS(","));
                }

                sprintf (buf, "%d", (int) idx);
                http_send (FS("{\"idx\":"));
                http_send (buf);

                sprintf (buf, "%d", (int) ir->protocol);
                http_send (FS(",\"protocol\":"));
                http_send (buf);

                sprintf (buf, "%d", (int) ir->address);
                http_send (FS(",\"address\":"));
                http_send (buf);

                sprintf (buf, "%d", (int) ir->command);
                http_send (FS(",\"command\":"));
                http_send (buf);

                http_send (FS("}"));
                first = 0;
            }
        }
    }

    http_send (FS("]}"));
    http_flush ();

    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * IR-Codes: eine Taste schreiben
 *
 * Alle vier Parameter sind Pflicht und werden vollstaendig geprueft, bevor irgendetwas geschieht.
 * Das ist die Lehre aus L70/L71: timer_set, overlay_display, overlay_delete und ambilight_timer_set
 * lasen alle atoi (http_get_param ("idx")) -- ein fehlender Parameter ergab 0, und 0 ist ein
 * gueltiger Index. overlay_delete ohne idx hat das erste Overlay geloescht und {"ok":true} gemeldet.
 * http_get_int_param() trennt "fehlt oder leer" von "ist 0" und weist Resttext hinter der Zahl ab.
 *
 * protocol 0 und 255 werden abgewiesen, nicht zurechtgebogen. Beides sind keine gueltigen
 * IRMP-Protokolle und tragen die Konvention "nie angelernt". Liesse man sie durch, machte ein
 * Restore eine Taste unbrauchbar, ohne dass es auffiele -- gezieltes Loeschen bietet diese API
 * bewusst nicht an.
 *
 * Das Kommando an den STM erzeugt set_ir_code_var() selbst: samt Maskierung auf feste Hexbreiten
 * (Befund L66, %02x ist eine MINDESTbreite) und samt Invalidierung des Puffers (AK9). Deshalb steht
 * hier kein eigenes Serial.printf ("CMD ...").
 *
 * Ein STM-Kommando und 5 Byte EEPROM pro Aufruf, rund 80 ms Hauptloop-Blockade. Ein Restore ueber
 * alle 20 Tasten sind 20 einzelne Requests, kein Burst.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static int
http_api_ir_code_set ()
{
    int     idx;
    int     protocol;
    int     address;
    int     command;

    if (! http_get_int_param ("idx", &idx))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "idx missing or not numeric");
        return 0;
    }

    if (! http_get_int_param ("protocol", &protocol))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "protocol missing or not numeric");
        return 0;
    }

    if (! http_get_int_param ("address", &address))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "address missing or not numeric");
        return 0;
    }

    if (! http_get_int_param ("command", &command))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "command missing or not numeric");
        return 0;
    }

    if (idx < 0 || idx >= (int) MAX_IR_CODES)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "idx out of range (0..19)");
        return 0;
    }

    if (protocol < 1 || protocol > 254)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "protocol out of range (1..254)");
        return 0;
    }

    if (address < 0 || address > 65535)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "address out of range (0..65535)");
        return 0;
    }

    if (command < 0 || command > 65535)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "command out of range (0..65535)");
        return 0;
    }

    /* Nach den Pruefungen oben kann das nicht fehlschlagen. Der Rueckgabewert wird trotzdem
     * ausgewertet: Ein verworfener Schreibvorgang darf nicht als {"ok":true} enden.
     */
    if (! set_ir_code_var ((uint_fast8_t) idx, (uint_fast8_t) protocol,
                           (uint_fast16_t) address, (uint_fast16_t) command))
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "idx out of range (0..19)");
        return 0;
    }

    http_json_ok ();

    return 0;
}

static int
http_api_temperature_display ()
{
    rpc (DISPLAY_TEMPERATURE_RPC_VAR);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();

    return 0;
}

/* Beide Korrektur-Endpunkte sind bis auf die beiden Variablen gleich. Sie benutzten
 * bis hierher blankes atoi: "?value=" ohne Inhalt wurde zu 0, die Antwort war
 * {"ok":true} - und eine eingemessene Kalibrierung war weg (L67). Dieselbe Klasse
 * wie L29/L48/L49, diese beiden waren dort uebersehen worden.
 *
 * Ausserhalb von -20..20 wird jetzt abgewiesen statt stillschweigend geklammert:
 * Wer 50 eintippt, hat sich vertan und soll es erfahren.
 */
static int
http_api_temperature_correction_set (NUM_VARIABLE index_var, NUM_VARIABLE correction_var)
{
    int temp_index;
    int old_correction;
    int temp_corr;

    if (! http_get_int_param ("value", &temp_corr))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or not numeric");
        return 0;
    }

    if (temp_corr != http_clamp_temp_correction (temp_corr))
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "correction out of range (-20..20)");
        return 0;
    }

    temp_index = (int) get_numvar (index_var);
    old_correction = (int) http_decode_temp_correction (get_numvar (correction_var));

    if (index_var == RTC_TEMP_INDEX_NUM_VAR)
    {
        http_rtc_temp_half_deg_correct (temp_corr - old_correction);           // A5: Index 49 im selben Schritt
    }

    temp_index -= (temp_corr - old_correction);

    if (temp_index < 0)
    {
        temp_index = 0;
    }
    else if (temp_index > 255)
    {
        temp_index = 255;
    }

    numvars[index_var] = temp_index;
    set_numvar (correction_var, http_encode_temp_correction (temp_corr));

    http_json_ok ();

    return 0;
}

static int
http_api_temperature_rtc_correction_set ()
{
    return http_api_temperature_correction_set (RTC_TEMP_INDEX_NUM_VAR, RTC_TEMP_CORRECTION_NUM_VAR);
}

static int
http_api_temperature_ds18xx_correction_set ()
{
    return http_api_temperature_correction_set (DS18XX_TEMP_INDEX_NUM_VAR, DS18XX_TEMP_CORRECTION_NUM_VAR);
}

static int
http_api_ldr_min_set ()
{
    rpc (LDR_MIN_VALUE_RPC_VAR);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();

    return 0;
}

/* Der ADC des STM ist 12 bit. Nach unten wurde geklammert, nach oben gar nicht:
 * ldr_max_value_set?value=99999 wurde angenommen (L68).
 */
#define HTTP_LDR_MAX_ADC_VALUE                  4095

/* Minimum und Maximum werden ohne Reihenfolgepruefung gesetzt; am Geraet stand
 * numvar17=14 ueber numvar18=12. ldr_poll_brightness (src/ldr/ldr.c:64) prueft
 * "ldr_max_value > ldr_min_value" und ueberspringt die Umrechnung sonst komplett -
 * die automatische Helligkeit steht dann dauerhaft auf Maximum, ohne Hinweis (L68).
 *
 * Abgewiesen wird die Verdrehung hier bewusst NICHT. Die beiden Messknoepfe der PWA
 * gehen ueber ldr_min_set/ldr_max_set als RPC direkt an den STM und erzeugen dieselbe
 * Verdrehung, ohne hier vorbeizukommen - eine Sperre nur im Wertsetzer wuerde also
 * nicht schuetzen, sondern nur den Weg zurueck verbauen und eine Sicherungsrueckgabe
 * auf halbem Weg abbrechen lassen. Stattdessen meldet die Antwort den Zustand mit;
 * "ok" bleibt true, unbekannte Felder stoeren die PWA nicht.
 */
static void
http_ldr_json_ok ()
{
    unsigned int ldr_min = get_numvar (LDR_MIN_VALUE_NUM_VAR);
    unsigned int ldr_max = get_numvar (LDR_MAX_VALUE_NUM_VAR);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true"));

    if (ldr_max <= ldr_min)
    {
        http_send (FS(",\"warning\":\"ldr_min_value >= ldr_max_value, automatic brightness inactive\""));
    }

    http_send (FS("}"));
    http_flush ();
}

static int
http_api_ldr_min_value_set ()
{
    int value;

    /* Beim L29-Fix uebersehen: "?value=" leer wurde zu 0 und hat die eingemessene
     * Untergrenze geloescht - mit Erfolgsmeldung (L67).
     */
    if (! http_get_int_param ("value", &value))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or not numeric");
        return 0;
    }

    if (value < 0 || value > HTTP_LDR_MAX_ADC_VALUE)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "value out of range (0..4095)");
        return 0;
    }

    set_numvar (LDR_MIN_VALUE_NUM_VAR, value);
    http_ldr_json_ok ();

    return 0;
}

static int
http_api_ldr_max_set ()
{
    rpc (LDR_MAX_VALUE_RPC_VAR);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();

    return 0;
}

static int
http_api_ldr_max_value_set ()
{
    int value;

    /* Gegenstueck zu ldr_min_value_set, siehe dort (L67/L68). */
    if (! http_get_int_param ("value", &value))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or not numeric");
        return 0;
    }

    if (value < 0 || value > HTTP_LDR_MAX_ADC_VALUE)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "value out of range (0..4095)");
        return 0;
    }

    set_numvar (LDR_MAX_VALUE_NUM_VAR, value);
    http_ldr_json_ok ();

    return 0;
}

static int
http_api_animation_mode_set ()
{
    int value;

    /* Beim L29-Fix uebersehen: Ein leeres oder nicht numerisches Feld wurde zu 0 und
     * schaltete die Animation ab - mit Erfolgsmeldung (L49).
     */
    if (! http_get_int_param ("value", &value))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or not numeric");
        return 0;
    }

    if (value < 0 || value >= (int) max_display_animation_variables)
    {
        http_json_error_range ("value", 0, max_display_animation_variables ? (int) max_display_animation_variables - 1 : 0);
        return 0;
    }

    set_numvar (ANIMATION_MODE_NUM_VAR, value);

    http_json_ok ();

    return 0;
}

static int
http_api_color_animation_mode_set ()
{
    int value;

    /* Wie animation_mode_set beim L29-Fix uebersehen: "abc" wurde zu 0 (L49). */
    if (! http_get_int_param ("value", &value))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or not numeric");
        return 0;
    }

    if (value < 0 || value >= MAX_COLOR_ANIMATION_VARIABLES)
    {
        http_json_error_range ("value", 0, MAX_COLOR_ANIMATION_VARIABLES - 1);
        return 0;
    }

    set_numvar (COLOR_ANIMATION_MODE_NUM_VAR, value);

    http_json_ok ();

    return 0;
}

static int
http_api_animation_profile_set ()
{
    int                 idx;
    int                 deceleration;
    uint_fast8_t        favourite = 0;
    DISPLAY_ANIMATION * da;

    /* Weder idx noch deceleration wurden bisher geprueft: Ein unzulaessiger Wert hat
     * das Profil unveraendert gelassen und trotzdem {"ok":true} gemeldet, ein leeres
     * Feld wurde zu 0 (Fehlerklasse von L30 und L29).
     */
    if (! http_get_int_param ("idx", &idx) || ! http_get_int_param ("deceleration", &deceleration))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "idx and deceleration required");
        return 0;
    }

    if (idx < 0 || idx >= (int) max_display_animation_variables)
    {
        http_json_error_range ("idx", 0, max_display_animation_variables ? (int) max_display_animation_variables - 1 : 0);
        return 0;
    }

    if (deceleration < ANIMATION_MIN_DECELERATION || deceleration > ANIMATION_MAX_DECELERATION)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "deceleration out of range (1..15)");
        return 0;
    }

    da = get_display_animation_var ((uint_fast8_t) idx);

    /* Die drei Geschwisterfunktionen pruefen hier, diese nicht -- dieselbe Asymmetrie wie
     * L179 und L272, zwei Wege zum selben Ziel und nur einer mit Schutz. AUSLOESBAR IST ES
     * HEUTE NICHT: max_display_animation_variables waechst in vars.cpp ausschliesslich
     * INNERHALB von "if (var_idx < MAX_DISPLAY_ANIMATION_VARIABLES)", bleibt also stets
     * <= MAX_DISPLAY_ANIMATION_VARIABLES, und genau daran haengt get_display_animation_var().
     * Diese Sicherheit steht aber in einer ANDEREN Datei und nirgends als Voraussetzung --
     * wer den Vorwaertsfilter oben auf das naheliegende MAX_... umstellt, erzeugt einen
     * Nullzeigerzugriff, ohne den Zusammenhang je gesehen zu haben.
     *
     * Die Pruefung steht VOR dem ersten Schreibzugriff: Ein Abbruch dahinter liesse die
     * Verzoegerung geschrieben und das Favoritenflag ungeschrieben zurueck.
     */
    if (! da)
    {
        http_json_error_range ("idx", 0, max_display_animation_variables ? (int) max_display_animation_variables - 1 : 0);
        return 0;
    }

    set_display_animation_deceleration ((uint_fast8_t) idx, (uint_fast8_t) deceleration);

    favourite = (! strcmp (http_get_param ("favourite"), "on")) ? 1 : 0;

    if (favourite)
    {
        set_display_animation_flags ((uint_fast8_t) idx, da->flags | ANIMATION_FLAG_FAVOURITE);
    }
    else
    {
        set_display_animation_flags ((uint_fast8_t) idx, da->flags & ~ANIMATION_FLAG_FAVOURITE);
    }

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();

    return 0;
}

static int
http_api_animation_profile_default ()
{
    int                 idx;
    DISPLAY_ANIMATION * da;

    /* Der blanke atoi machte aus einem fehlenden idx die 0 und setzte damit Profil 0
     * zurueck, ein unzulaessiger idx meldete stillen Erfolg (L51, L40).
     */
    if (! http_get_int_param ("idx", &idx))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "idx required");
        return 0;
    }

    if (idx < 0 || idx >= (int) max_display_animation_variables)
    {
        http_json_error_range ("idx", 0, max_display_animation_variables ? (int) max_display_animation_variables - 1 : 0);
        return 0;
    }

    da = get_display_animation_var ((uint_fast8_t) idx);

    if (! da)
    {
        http_json_error_range ("idx", 0, max_display_animation_variables ? (int) max_display_animation_variables - 1 : 0);
        return 0;
    }

    /* Das Favoritenflag wird hier bewusst nicht mehr gesetzt (L51): Der ESP kennt nur
     * default_deceleration als Vorgabe, einen Vorgabewert fuer die Flags gibt es in
     * DISPLAY_ANIMATION nicht. Das bisherige "| ANIMATION_FLAG_FAVOURITE" hat aus einer
     * Animation, die nie Favorit war - Profil 0 "None" etwa -, beim Zuruecksetzen einen
     * Favoriten gemacht. Gesetzt und geloescht wird das Flag ueber animation_profile_set.
     */
    set_display_animation_deceleration ((uint_fast8_t) idx, da->default_deceleration);

    http_json_ok ();

    return 0;
}

static int
http_api_color_animation_profile_set ()
{
    int idx;
    int deceleration;

    /* Stiller Nichtstun-Pfad mit Erfolgsmeldung, und ein leeres Feld wurde zur
     * schnellsten Stufe 0 (Fehlerklasse von L30 und L29).
     */
    if (! http_get_int_param ("idx", &idx) || ! http_get_int_param ("deceleration", &deceleration))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "idx and deceleration required");
        return 0;
    }

    if (idx < 0 || idx >= MAX_COLOR_ANIMATION_VARIABLES)
    {
        http_json_error_range ("idx", 0, MAX_COLOR_ANIMATION_VARIABLES - 1);
        return 0;
    }

    if (deceleration < 0 || deceleration > COLOR_ANIMATION_MAX_DECELERATION)
    {
        http_json_error_range ("deceleration", 0, COLOR_ANIMATION_MAX_DECELERATION);
        return 0;
    }

    set_color_animation_deceleration ((COLOR_ANIMATION_VARIABLE) idx, (uint_fast8_t) deceleration);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();

    return 0;
}

static int
http_api_color_animation_profile_default ()
{
    int                 idx;
    COLOR_ANIMATION *   ca;

    /* Gleicher stiller idx-Pfad wie bei animation_profile_default (L51, L40). */
    if (! http_get_int_param ("idx", &idx))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "idx required");
        return 0;
    }

    if (idx < 0 || idx >= MAX_COLOR_ANIMATION_VARIABLES)
    {
        http_json_error_range ("idx", 0, MAX_COLOR_ANIMATION_VARIABLES - 1);
        return 0;
    }

    ca = get_color_animation_var ((COLOR_ANIMATION_VARIABLE) idx);

    if (! ca)
    {
        http_json_error_range ("idx", 0, MAX_COLOR_ANIMATION_VARIABLES - 1);
        return 0;
    }

    set_color_animation_deceleration ((COLOR_ANIMATION_VARIABLE) idx, ca->default_deceleration);

    http_json_ok ();

    return 0;
}

static int
http_api_display_dim_level_set ()
{
    int idx;
    int value;

    /* Ein fehlender idx wurde ueber atoi("") zu 0 und schrieb damit auf die falsche
     * Dimmstufe - mit Erfolgsmeldung (L50). Ein unzulaessiger idx tat gar nichts und
     * meldete ebenfalls Erfolg (L40).
     */
    if (! http_get_int_param ("idx", &idx) || ! http_get_int_param ("value", &value))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "idx and value required");
        return 0;
    }

    if (idx < 0 || idx > MAX_BRIGHTNESS)
    {
        http_json_error_range ("idx", 0, MAX_BRIGHTNESS);
        return 0;
    }

    if (value < 0 || value > 15)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "value out of range (0..15)");
        return 0;
    }

    set_num8_array (DISPLAY_DIMMED_DISPLAY_COLORS, idx, value);

    http_json_ok ();

    return 0;
}

static int
http_api_ambilight_dim_level_set ()
{
    int idx;
    int value;

    /* Wortgleich zu display_dim_level_set: fehlender idx traf Stufe 0 (L50),
     * unzulaessiger idx meldete stillen Erfolg (L40).
     */
    if (! http_get_int_param ("idx", &idx) || ! http_get_int_param ("value", &value))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "idx and value required");
        return 0;
    }

    if (idx < 0 || idx > MAX_BRIGHTNESS)
    {
        http_json_error_range ("idx", 0, MAX_BRIGHTNESS);
        return 0;
    }

    if (value < 0 || value > 15)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "value out of range (0..15)");
        return 0;
    }

    set_num8_array (DISPLAY_DIMMED_AMBILIGHT_COLORS, idx, value);

    http_json_ok ();

    return 0;
}

static int
http_api_tft_flags_set ()
{
    uint_fast8_t flags = 0;
    int          rgb, hflip, vflip;

    // E12: ein fehlender Parameter loeschte bisher sein Flag - jetzt Abweisung, nichts geschrieben
    if ((rgb = http_get_on_off_required ("rgb")) < 0 || (hflip = http_get_on_off_required ("hflip")) < 0 || (vflip = http_get_on_off_required ("vflip")) < 0)
    {
        return 0;
    }

    if (rgb)
    {
        flags |= SSD1963_GLOBAL_FLAGS_RGB_ORDER;
    }

    if (hflip)
    {
        flags |= SSD1963_GLOBAL_FLAGS_FLIP_HORIZONTAL;
    }

    if (vflip)
    {
        flags |= SSD1963_GLOBAL_FLAGS_FLIP_VERTICAL;
    }

    set_numvar (SSD1963_FLAGS_NUM_VAR, flags);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();

    return 0;
}

static int
http_api_display_use_rgbw_set ()
{
    uint_fast8_t use_rgbw = http_get_on_off_value ("value", get_numvar (DISPLAY_USE_RGBW_NUM_VAR));

    set_numvar (DISPLAY_USE_RGBW_NUM_VAR, use_rgbw ? 1 : 0);
    http_json_ok ();

    return 0;
}

static int
http_api_ambilight_brightness_set ()
{
    int brightness;

    /* Leer heisst nicht 0 - sonst dunkelt ein leeres Feld das Ambilight ab (L29). */
    if (! http_get_int_param ("value", &brightness))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or not numeric");
        return 0;
    }

    /* Abweisen statt klemmen (L186); die Grenze bleibt gegen die stille Verkuerzung
     * in set_numvar stehen (L198).
     */
    if (brightness < 0 || brightness > 15)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "value out of range (0..15)");
        return 0;
    }

    set_numvar (AMBILIGHT_BRIGHTNESS_NUM_VAR, brightness);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"ambilight_brightness\":"));
    char buf[8];
    sprintf (buf, "%d", brightness);
    http_send (buf);
    http_send (FS("}"));
    http_flush ();

    return 0;
}

static int
http_api_ambilight_mode_set ()
{
    int mode;

    /* Leer heisst nicht 0 - sonst wechselt der Ambilight-Modus ungewollt (L29). */
    if (! http_get_int_param ("value", &mode))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or not numeric");
        return 0;
    }

    /* Abweisen statt klemmen (L186). MAX_AMBILIGHT_MODE_VARIABLES ist 5, der Bereich
     * also 0..4; der Vergleich benutzt weiter das Symbol, nur der Text nennt die Zahl.
     */
    if (mode < 0 || mode >= MAX_AMBILIGHT_MODE_VARIABLES)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "value out of range (0..4)");
        return 0;
    }

    set_numvar (AMBILIGHT_MODE_NUM_VAR, mode);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"ambilight_mode\":"));
    char buf[8];
    sprintf (buf, "%d", mode);
    http_send (buf);
    http_send (FS("}"));
    http_flush ();

    return 0;
}

static int
http_api_ambilight_leds_set ()
{
    int leds;

    /* Ein leeres Feld wuerde die LED-Kette auf 0 setzen und das Ambilight
     * stilllegen (L29).
     */
    if (! http_get_int_param ("value", &leds))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or not numeric");
        return 0;
    }

    /* Abweisen statt klemmen (L186); die Grenze bleibt gegen die stille Verkuerzung
     * in set_numvar stehen (L198).
     */
    if (leds < 0 || leds > 999)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "value out of range (0..999)");
        return 0;
    }

    set_numvar (AMBILIGHT_LEDS_NUM_VAR, leds);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"ambilight_leds\":"));
    char buf[8];
    sprintf (buf, "%d", leds);
    http_send (buf);
    http_send (FS("}"));
    http_flush ();

    return 0;
}

static int
http_api_ambilight_offset_set ()
{
    int offset;

    /* Leer heisst nicht 0 - sonst verschiebt ein leeres Feld den Startpunkt der
     * Kette ungewollt auf Anfang (L29).
     */
    if (! http_get_int_param ("value", &offset))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or not numeric");
        return 0;
    }

    /* Abweisen statt klemmen (L186); die Grenze bleibt gegen die stille Verkuerzung
     * in set_numvar stehen (L198).
     */
    if (offset < 0 || offset > 999)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "value out of range (0..999)");
        return 0;
    }

    set_numvar (AMBILIGHT_OFFSET_NUM_VAR, offset);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"ambilight_offset\":"));
    char buf[8];
    sprintf (buf, "%d", offset);
    http_send (buf);
    http_send (FS("}"));
    http_flush ();

    return 0;
}

static int
http_api_ambilight_mode_profile_set ()
{
    int idx;
    int deceleration;

    /* Ein idx ausserhalb des Bereichs hat bisher gar nichts bewirkt und trotzdem
     * {"ok":true} gemeldet (Fehlerklasse von L30). deceleration wurde geklemmt und
     * ebenfalls als Erfolg gemeldet - das ist der belegte Ausgangsfall von L186 und
     * wird hier abgewiesen.
     */
    if (! http_get_int_param ("idx", &idx) || ! http_get_int_param ("deceleration", &deceleration))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "idx and deceleration required");
        return 0;
    }

    if (idx < 0 || idx >= MAX_AMBILIGHT_MODE_VARIABLES)
    {
        http_json_error_range ("idx", 0, MAX_AMBILIGHT_MODE_VARIABLES - 1);
        return 0;
    }

    /* Die Grenze bleibt stehen: set_ambilight_mode_deceleration schickt den Wert mit
     * "%02x", und %02x ist eine Mindest-, keine Hoechstbreite - ueber 255 entstehen drei
     * Hexziffern und verschieben das ganze Kommando auf der Bruecke (L66, L198).
     */
    if (deceleration < 0 || deceleration > AMBILIGHT_MODE_MAX_DECELERATION)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "deceleration out of range (0..15)");
        return 0;
    }

    set_ambilight_mode_deceleration ((AMBILIGHT_MODE_VARIABLE) idx, (uint_fast8_t) deceleration);

    http_json_ok ();

    return 0;
}

static int
http_api_ambilight_mode_profile_default ()
{
    int                 idx;
    AMBILIGHT_MODE *    am;

    /* Dritter Fall derselben Machart wie L51/L40, in den Befunden nicht eigens genannt:
     * fehlender idx traf ueber atoi("") Profil 0, unzulaessiger idx meldete Erfolg.
     */
    if (! http_get_int_param ("idx", &idx))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "idx required");
        return 0;
    }

    if (idx < 0 || idx >= MAX_AMBILIGHT_MODE_VARIABLES)
    {
        http_json_error_range ("idx", 0, MAX_AMBILIGHT_MODE_VARIABLES - 1);
        return 0;
    }

    am = get_ambilight_mode_var ((AMBILIGHT_MODE_VARIABLE) idx);

    if (! am)
    {
        http_json_error_range ("idx", 0, MAX_AMBILIGHT_MODE_VARIABLES - 1);
        return 0;
    }

    set_ambilight_mode_deceleration ((AMBILIGHT_MODE_VARIABLE) idx, am->default_deceleration);

    http_json_ok ();

    return 0;
}

static void
http_json_ok ()
{
    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();
}

/* Fehlerantwort in derselben Form wie http_api_app_file_upload: HTTP 200 mit ok=false.
 * Ein echter HTTP-Fehlerstatus wuerde in der Legacy-Oberflaeche als Verbindungsabbruch
 * erscheinen, statt die Ursache zu zeigen.
 */
static void
http_json_error (unsigned int error_code, const char * detail)
{
    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":false,\"error\":"));
    http_send (String (error_code).c_str ());
    http_send (FS(",\"detail\":\""));
    http_send_json_escaped (detail ? detail : "");
    http_send (FS("\"}"));
    http_flush ();
}

/* Trennt "Parameter fehlt oder ist leer" von "Parameter ist 0". Das bisherige
 * atoi (value ? value : "0") konnte das nicht: Ein leeres Helligkeitsfeld hat das
 * Display dunkel geschaltet, ein leeres LED-Feld die Kette auf 0 gesetzt (L29).
 * Resttext hinter der Zahl gilt ebenfalls als ungueltig - atoi haette daraus still
 * eine 0 gemacht.
 */
static uint_fast8_t
http_get_int_param (const char * name, int * valuep)
{
    char *  value = http_get_param (name);
    char *  endp;
    long    parsed;

    if (! *value)                                           // nie NULL, siehe Vertrag bei http_get_param (C20/L199)
    {
        return 0;
    }

    parsed = strtol (value, &endp, 10);

    if (endp == value || *endp)
    {
        return 0;
    }

    *valuep = (int) parsed;

    return 1;
}

/* Fuer Felder, deren Fehlen absichtlich einen Rueckfallwert bedeutet - interval,
 * duration, month, day und days des Overlays. Fehlt der Parameter, bleibt *valuep
 * unberuehrt und der Aufrufer behaelt seinen Vorgabewert. Ist er da, muss er eine
 * Zahl im Bereich sein; "abc" oder 300 gelten als Fehler statt still als 0 bzw. als
 * auf zwei Hexziffern verkuerzter Wert (L66).
 */
static uint_fast8_t
http_get_opt_int_param (const char * name, int * valuep, int lo, int hi)
{
    char *  value = http_get_param (name);
    char *  endp;
    long    parsed;

    if (! *value)                                           // nie NULL, siehe Vertrag bei http_get_param (C20/L199)
    {
        return 1;
    }

    parsed = strtol (value, &endp, 10);

    if (endp == value || *endp || parsed < (long) lo || parsed > (long) hi)
    {
        return 0;
    }

    *valuep = (int) parsed;

    return 1;
}

/* Gegenstueck zu http_get_int_param fuer Textfelder: "leer" hiess bisher "loeschen",
 * und ein leeres Zeitserver- oder AppID-Feld hat der Uhr mit gruenem "Gespeichert" die
 * Zeitquelle bzw. das Wetter genommen (L48). "Fehlt" und "leer" sind hier nicht zu
 * trennen - http_get_param liefert fuer einen fehlenden Parameter denselben leeren
 * String -, also gilt beides als Fehler.
 */
static uint_fast8_t
http_get_string_param (const char * name, char ** valuep)
{
    char * value = http_get_param (name);

    if (! *value)
    {
        return 0;
    }

    *valuep = value;

    return 1;
}

/* C22/L206: Zeichenketten wurden bisher STILL auf die Breite der Variablen gekuerzt und
 * das Ergebnis als {"ok":true} gemeldet -- AppID 33 auf 32, Zeitserver 17 auf 16,
 * Update-Host 64 auf 63. Beim Update-Host ist das kein Schoenheitsfehler: Ein gekuerzter
 * Hostname zeigt auf einen ANDEREN Server, und genau das war L124.
 *
 * Gezaehlt werden BYTES, nicht Zeichen -- die Grenze der Variablen ist eine Pufferbreite.
 * Die Kuerzung in set_strvar() bleibt als letztes Netz stehen; sie wird ab hier nur nicht
 * mehr erreicht.
 *
 * DESHALB SAGT DIE MELDUNG "bytes" UND NICHT "characters", und das ist keine Wortklauberei:
 * Die Oberflaeche begrenzt dieselben Felder ueber maxlength auf ZEICHEN. Ein Tickertext aus
 * 32 Umlauten sind 64 Byte -- das Eingabefeld laesst ihn zu, das Geraet weist ihn ab, und
 * eine Meldung "max. 32 characters" waere bei genau 32 Zeichen im Feld nicht aufloesbar.
 * Die Oberflaeche muss vor dem Absenden in Byte rechnen; bis dahin ist "bytes" wenigstens
 * wahr. Wer hier je "characters" zurueckschreibt, macht die Meldung wieder irrefuehrend.
 *
 * Die Fehlerantwort ist bereits gesendet, wenn 0 zurueckkommt; der Aufrufer bricht nur
 * noch ab -- dieselbe Form wie bei http_get_color_component().
 */
/* Die Regel selbst, ohne Antwort - fuer Wege, die keine JSON-Fehlerantwort senden koennen, wie
 * die Legacy-WLAN-Seite (Review F.6, A4). http_check_strvar_len () benutzt sie ebenfalls, damit
 * es genau EINE Byte-Regel gibt.
 */
static uint_fast8_t
http_strvar_len_ok (const char * value, unsigned int maxlen)
{
    return strlen (value) <= maxlen ? 1 : 0;
}

static uint_fast8_t
http_check_strvar_len (const char * name, const char * value, unsigned int maxlen)
{
    if (! http_strvar_len_ok (value, maxlen))
    {
        char detail[64];

        snprintf (detail, sizeof (detail), "%s too long (max. %u bytes)", name, maxlen);
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, detail);
        return 0;
    }

    return 1;
}

/* C18/L186: Fuenfzehn Abweisungen nannten den Bereich nicht, den sie pruefen -- "idx out of
 * range" ohne jede Zahl. Die Oberflaeche zeigt den detail-Text woertlich an (L201), der
 * Nutzer erfuhr also weder den erlaubten Bereich noch, welcher Wert gemeint war.
 *
 * snprintf in einen Stackpuffer, KEIN String (L175): Der Heap ist der Engpass dieser
 * Laufzeit, und drei der Grenzen sind Laufzeitwerte bzw. Aufzaehlungsenden, die sich im
 * Text nicht als Literal schreiben lassen. Eine Funktion statt fuenfzehn Literale spart
 * zusaetzlich Flash.
 */
static void
http_json_error_range (const char * name, int lo, int hi)
{
    char detail[64];

    snprintf (detail, sizeof (detail), "%s out of range (%d..%d)", name, lo, hi);
    http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, detail);
}

/* Laenge eines Monats inklusive Schaltjahr. Ohne sie nimmt datetime_set den
 * 31. Februar widerspruchslos an (L32).
 */
static uint_fast8_t
http_days_in_month (int year, int month)
{
    static const uint_fast8_t days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (month < 1 || month > 12)
    {
        return 0;
    }

    if (month == 2 && ((year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0)))
    {
        return 29;
    }

    return days[month - 1];
}

static uint_fast8_t
http_get_on_off_value (const char * param, uint_fast8_t current_value)
{
    char * value = http_get_param (param);

    if (! strcmp (value, "on"))
    {
        return 1;
    }
    else if (! strcmp (value, "off"))
    {
        return 0;
    }

    return current_value;
}

/* E12: wie http_get_on_off_value(), aber ohne Vorgabewert. Liefert 1 fuer "on", 0 fuer "off";
 * fehlt der Parameter, gibt es Kennung 1, steht etwas anderes darin, Kennung 2 - dann ist die
 * Fehlerantwort gesendet und das Ergebnis -1. Fuer Setter, die mehrere Flags in EINEM Wert
 * schreiben: Dort hiess "fehlt" bisher "aus", und ein unvollstaendiger Request loeschte Flags.
 */
static int
http_get_on_off_required (const char * param)
{
    uint_fast8_t    value = http_get_on_off_value (param, 2);                        // 2: weder "on" noch "off"
    char            detail[40];

    if (value != 2)
    {
        return value;
    }

    snprintf (detail, sizeof (detail), "%s must be on or off", param);
    http_json_error (*http_get_param (param) ? HTTP_API_ERROR_OUT_OF_RANGE : HTTP_API_ERROR_MISSING_VALUE, detail);
    return -1;
}

/* Gibt HTTP_COLOR_COMPONENT_OK zurueck, wenn der Farbanteil im Request stand und im
 * Bereich lag, HTTP_COLOR_COMPONENT_ABSENT wenn er fehlte, und
 * HTTP_COLOR_COMPONENT_REJECTED wenn er ausserhalb 0..63 lag. Im letzten Fall ist die
 * Fehlerantwort bereits gesendet und der Aufrufer bricht nur noch ab.
 *
 * Das bisherige atoi (value ? value : "0") konnte "fehlt" nicht von "0" trennen: Ein
 * Request ohne "white" hat den Weissanteil geloescht, ein Request nur mit "red" die
 * Farbe bis auf Rot schwarz gemacht - jedesmal mit Erfolgsmeldung (L37). Ein Anteil
 * ueber 63 wurde geklemmt und ebenfalls als Erfolg gemeldet (L186).
 */
#define HTTP_COLOR_COMPONENT_ABSENT         0
#define HTTP_COLOR_COMPONENT_OK             1
#define HTTP_COLOR_COMPONENT_REJECTED       2

static uint_fast8_t
http_get_color_component (const char * param, uint8_t * valuep)
{
    int     component;
    char    detail[40];                                                             // laengste Form: "green out of range (0..63)" = 26 Zeichen

    if (! http_get_int_param (param, &component))
    {
        return HTTP_COLOR_COMPONENT_ABSENT;
    }

    /* Die Grenze bleibt stehen - der Anteil geht als Byte an den STM (L198). */
    if (component < 0 || component > 63)
    {
        snprintf (detail, sizeof (detail), "%s out of range (0..63)", param);
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, detail);
        return HTTP_COLOR_COMPONENT_REJECTED;
    }

    *valuep = (uint8_t) component;

    return HTTP_COLOR_COMPONENT_OK;
}

static int
http_api_set_dsp_color (DSP_COLOR_VARIABLE var)
{
    DSP_COLORS   rgbw = { 0, 0, 0, 0 };
    uint_fast8_t use_rgbw = get_numvar (DISPLAY_USE_RGBW_NUM_VAR);
    uint_fast8_t found = 0;
    uint_fast8_t rtc;

    /* Fehlende Anteile behalten den eingestellten Wert, statt auf 0 zu fallen (L37).
     * Ein abgewiesener Anteil bricht den ganzen Aufruf ab, bevor irgendetwas
     * geschrieben wird - sonst entstuende eine halb uebernommene Farbe.
     */
    get_dsp_color_var (var, &rgbw);

    rtc = http_get_color_component ("red", &rgbw.red);

    if (rtc == HTTP_COLOR_COMPONENT_REJECTED)
    {
        return 0;
    }

    found |= rtc;

    rtc = http_get_color_component ("green", &rgbw.green);

    if (rtc == HTTP_COLOR_COMPONENT_REJECTED)
    {
        return 0;
    }

    found |= rtc;

    rtc = http_get_color_component ("blue", &rgbw.blue);

    if (rtc == HTTP_COLOR_COMPONENT_REJECTED)
    {
        return 0;
    }

    found |= rtc;

    if (use_rgbw)
    {
        rtc = http_get_color_component ("white", &rgbw.white);

        if (rtc == HTTP_COLOR_COMPONENT_REJECTED)
        {
            return 0;
        }

        found |= rtc;
    }
    else
    {
        rgbw.white = 0;
    }

    /* Ohne einen einzigen Anteil gaebe es nichts zu schreiben - der Aufruf haette nur
     * ein STM-Kommando samt EEPROM-Schreibzyklus gekostet und Erfolg gemeldet.
     */
    if (! found)
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "red, green or blue required");
        return 0;
    }

    set_dsp_color_var (var, &rgbw, use_rgbw);
    http_json_ok ();

    return 0;
}

static int
http_api_display_color_set ()
{
    return http_api_set_dsp_color (DISPLAY_DSP_COLOR_VAR);
}

static int
http_api_ambilight_color_set ()
{
    return http_api_set_dsp_color (AMBILIGHT_DSP_COLOR_VAR);
}

static int
http_api_marker_color_set ()
{
    return http_api_set_dsp_color (AMBILIGHT_MARKER_DSP_COLOR_VAR);
}

static int
http_api_live_display_color ()
{
    DSP_COLORS rgbw;
    char       buf[160];

    get_dsp_color_var (DISPLAY_DSP_COLOR_VAR, &rgbw);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    sprintf (buf,
             FS("{\"ok\":true,\"red\":%d,\"green\":%d,\"blue\":%d,\"white\":%d}"),
             rgbw.red,
             rgbw.green,
             rgbw.blue,
             rgbw.white);
    http_send (buf);
    http_flush ();

    return 0;
}

static int
http_api_sync_ambilight_set ()
{
    uint_fast8_t flags = get_numvar (DISPLAY_FLAGS_NUM_VAR);

    if (http_get_on_off_value ("value", flags & DISPLAY_FLAGS_SYNC_AMBILIGHT))
    {
        flags |= DISPLAY_FLAGS_SYNC_AMBILIGHT;
    }
    else
    {
        flags &= ~DISPLAY_FLAGS_SYNC_AMBILIGHT;
    }

    set_numvar (DISPLAY_FLAGS_NUM_VAR, flags);
    http_json_ok ();

    return 0;
}

static int
http_api_sync_markers_set ()
{
    uint_fast8_t flags = get_numvar (DISPLAY_FLAGS_NUM_VAR);

    if (http_get_on_off_value ("value", flags & DISPLAY_FLAGS_SYNC_CLOCK_MARKERS))
    {
        flags |= DISPLAY_FLAGS_SYNC_CLOCK_MARKERS;
    }
    else
    {
        flags &= ~DISPLAY_FLAGS_SYNC_CLOCK_MARKERS;
    }

    set_numvar (DISPLAY_FLAGS_NUM_VAR, flags);
    http_json_ok ();

    return 0;
}

static int
http_api_fade_clock_seconds_set ()
{
    uint_fast8_t flags = get_numvar (DISPLAY_FLAGS_NUM_VAR);

    if (http_get_on_off_value ("value", flags & DISPLAY_FLAGS_FADE_CLOCK_SECONDS))
    {
        flags |= DISPLAY_FLAGS_FADE_CLOCK_SECONDS;
    }
    else
    {
        flags &= ~DISPLAY_FLAGS_FADE_CLOCK_SECONDS;
    }

    set_numvar (DISPLAY_FLAGS_NUM_VAR, flags);
    http_json_ok ();

    return 0;
}

static int
http_api_ambilight_markers_set ()
{
    AMBILIGHT_MODE * am = get_ambilight_mode_var (CLOCK_AMBILIGHT_MODE_VAR);
    uint_fast8_t     flags = am ? am->flags : 0;

    if (http_get_on_off_value ("value", flags & AMBILIGHT_FLAG_SECONDS_MARKER))
    {
        flags |= AMBILIGHT_FLAG_SECONDS_MARKER;
    }
    else
    {
        flags &= ~AMBILIGHT_FLAG_SECONDS_MARKER;
    }

    set_ambilight_mode_flags (CLOCK_AMBILIGHT_MODE_VAR, flags);
    http_json_ok ();

    return 0;
}

static int
http_api_ambilight_online_set ()
{
    char * value = http_get_param ("value");

    if (! strcmp (value, "on"))
    {
        set_numvar (AMBILIGHT_IS_UP_NUM_VAR, 1);
    }
    else if (! strcmp (value, "off"))
    {
        set_numvar (AMBILIGHT_IS_UP_NUM_VAR, 0);
    }

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"ambilight_online\":\""));
    http_send (get_numvar (AMBILIGHT_IS_UP_NUM_VAR) ? "on" : "off");
    http_send (FS("\"}"));
    http_flush ();

    return 0;
}

static int
http_api_dfplayer_volume_set ()
{
    int volume;

    /* Leer heisst nicht 0 - sonst stellt ein leeres Feld den Ton stumm (L29). */
    if (! http_get_int_param ("value", &volume))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or not numeric");
        return 0;
    }

    /* Abweisen statt klemmen (L186). DFPLAYER_MAX_VOLUME ist 30; der Vergleich benutzt
     * weiter das Symbol, nur der Text nennt die Zahl.
     */
    if (volume < 0 || volume > DFPLAYER_MAX_VOLUME)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "value out of range (0..30)");
        return 0;
    }

    set_numvar (DFPLAYER_VOLUME_NUM_VAR, volume);

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"dfplayer_volume\":"));
    char buf[8];
    sprintf (buf, "%d", volume);
    http_send (buf);
    http_send (FS("}"));
    http_flush ();

    return 0;
}

static int
http_api_dfplayer_mode_set ()
{
    int mode;

    /* Leer heisst nicht DFPLAYER_MODE_NONE - sonst schaltet ein leeres Feld die
     * Tonausgabe ganz ab (L29).
     */
    if (! http_get_int_param ("value", &mode))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or not numeric");
        return 0;
    }

    /* Abweisen statt klemmen (L186). DFPLAYER_MODE_NONE ist 0, DFPLAYER_MODE_SPEAK ist 2;
     * der Vergleich benutzt weiter die Symbole, nur der Text nennt die Zahlen.
     */
    if (mode < DFPLAYER_MODE_NONE || mode > DFPLAYER_MODE_SPEAK)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "value out of range (0..2)");
        return 0;
    }

    set_numvar (DFPLAYER_MODE_NUM_VAR, mode);
    http_json_ok ();

    return 0;
}

static int
http_api_dfplayer_bell_flags_set ()
{
    uint_fast8_t flags = DFPLAYER_MODE_BELL_FLAG_NONE;
    int          m15, m30, m45;

    // E12: ein fehlender Parameter loeschte bisher sein Flag - jetzt Abweisung, nichts geschrieben
    if ((m15 = http_get_on_off_required ("m15")) < 0 || (m30 = http_get_on_off_required ("m30")) < 0 || (m45 = http_get_on_off_required ("m45")) < 0)
    {
        return 0;
    }

    if (m15)
    {
        flags |= DFPLAYER_MODE_BELL_FLAG_15;
    }
    if (m30)
    {
        flags |= DFPLAYER_MODE_BELL_FLAG_30;
    }
    if (m45)
    {
        flags |= DFPLAYER_MODE_BELL_FLAG_45;
    }

    set_numvar (DFPLAYER_BELL_FLAGS_NUM_VAR, flags);
    http_json_ok ();

    return 0;
}

static int
http_api_dfplayer_speak_cycle_set ()
{
    int speak_cycle;

    /* Leer heisst nicht 0 - eine 0 schaltet die Sprachansage ab (L29). */
    if (! http_get_int_param ("value", &speak_cycle))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "value missing or not numeric");
        return 0;
    }

    /* Abweisen statt klemmen (L186); die Grenze bleibt gegen die stille Verkuerzung
     * in set_numvar stehen (L198).
     */
    if (speak_cycle < 0 || speak_cycle > 255)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "value out of range (0..255)");
        return 0;
    }

    set_numvar (DFPLAYER_SPEAK_CYCLE_NUM_VAR, speak_cycle);
    http_json_ok ();

    return 0;
}

static int
http_api_dfplayer_silence_start_set ()
{
    int hour;
    int minute;

    /* Fehlende oder unzulaessige Zeit hat bisher gar nichts bewirkt und trotzdem
     * {"ok":true} gemeldet - die Oberflaeche zeigte "gespeichert" an, obwohl im
     * Geraet nichts stand (Fehlerklasse von L30).
     */
    if (! http_get_int_param ("hour", &hour) || ! http_get_int_param ("minute", &minute))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "hour and minute required");
        return 0;
    }

    if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "hour out of range (0..23) or minute out of range (0..59)");
        return 0;
    }

    set_numvar (DFPLAYER_SILENCE_START_NUM_VAR, hour * 60 + minute);

    http_json_ok ();

    return 0;
}

static int
http_api_dfplayer_silence_stop_set ()
{
    int hour;
    int minute;

    /* Wie bei silence_start: stiller Nichtstun-Pfad mit Erfolgsmeldung. */
    if (! http_get_int_param ("hour", &hour) || ! http_get_int_param ("minute", &minute))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "hour and minute required");
        return 0;
    }

    if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "hour out of range (0..23) or minute out of range (0..59)");
        return 0;
    }

    set_numvar (DFPLAYER_SILENCE_STOP_NUM_VAR, hour * 60 + minute);

    http_json_ok ();

    return 0;
}

static int
http_api_dfplayer_play ()
{
    int folder;
    int track;

    /* Bisher rohes atoi (http_get_param (...)). http_get_param liefert fuer einen
     * fehlenden Parameter einen leeren String und nie NULL - ein Aufruf ohne folder und
     * track hat damit still Ordner 0, Titel 0 gespielt und Erfolg gemeldet.
     * http_get_int_param trennt "fehlt oder leer" von "ist 0" (L29).
     */
    if (! http_get_int_param ("folder", &folder) || ! http_get_int_param ("track", &track))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "folder and track required");
        return 0;
    }

    /* Abweisen statt klemmen (L186). Die Grenzen bleiben stehen: Beide Werte gehen als
     * folder << 8 | track in eine 16-Bit-Variable, und set_numvar verkuerzt still (L198).
     */
    if (folder < 0 || folder > 255)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "folder out of range (0..255)");
        return 0;
    }

    if (track < 0 || track > 255)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "track out of range (0..255)");
        return 0;
    }

    set_numvar (DFPLAYER_PLAY_FOLDER_TRACK_NUM_VAR, folder << 8 | track);
    http_json_ok ();

    return 0;
}

/* C17/L197: Drei Maengel, und fuer jeden stand die Loesung bereits in
 * http_api_timer_set_common() -- ABGESCHRIEBEN, nicht neu erfunden, dieselben drei
 * Pruefungen in derselben Reihenfolge und derselben Form:
 *
 *   (1) idx wurde mit atoi gelesen und in einem if OHNE else geprueft. Ein Index
 *       ausserhalb uebersprang den ganzen Block, http_json_ok() feuerte trotzdem; ein
 *       FEHLENDER idx wurde ueber atoi("") zu 0 und ueberschrieb Alarm 0.
 *   (2) from und to wurden MASKIERT statt geprueft: from=9 ergab 9<<3 = 0x48, maskiert
 *       0x08 -- also Montag.
 *   (3) hour und minute wurden gar nicht geprueft: hour=99&minute=99 ergab 6039 Minuten,
 *       ein Tag hat 1440. set_alarm_time_var() verkuerzte danach still auf 16 Bit.
 *
 * Der so gesetzte Alarm stand anschliessend als aktiv in der Liste und konnte NIE
 * ausloesen -- kein Fehler, keine Meldung. Die Wochentagsgrenze ist HTTP_MAX_WEEKDAY wie
 * beim Zwilling; die Maske traegt zwar 3 Bit, aber den Wert 7 gibt es als Tag nicht.
 */
static int
http_api_dfplayer_alarm_set ()
{
    int             idx;
    int             from_day;
    int             to_day;
    int             hour;
    int             minute;
    ALARM_TIME *    at;
    uint_fast8_t    flags;

    if (! http_get_int_param ("idx", &idx) ||
        ! http_get_int_param ("from", &from_day) ||
        ! http_get_int_param ("to", &to_day) ||
        ! http_get_int_param ("hour", &hour) ||
        ! http_get_int_param ("minute", &minute))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "idx, from, to, hour and minute required");
        return 0;
    }

    if (idx < 0 || idx >= MAX_ALARM_TIME_VARIABLES)
    {
        http_json_error_range ("idx", 0, MAX_ALARM_TIME_VARIABLES - 1);
        return 0;
    }

    if (from_day < 0 || from_day > HTTP_MAX_WEEKDAY || to_day < 0 || to_day > HTTP_MAX_WEEKDAY)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "from and to out of range (0..6)");
        return 0;
    }

    if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "hour out of range (0..23) or minute out of range (0..59)");
        return 0;
    }

    at = get_alarm_time_var ((ALARM_TIME_VARIABLE) idx);
    flags = at ? at->flags : 0;

    if (http_get_on_off_value ("active", 0))
    {
        flags |= ALARM_TIME_FLAG_ACTIVE;
    }
    else
    {
        flags &= ~ALARM_TIME_FLAG_ACTIVE;
    }

    flags &= ~(ALARM_TIME_FROM_DAY_MASK | ALARM_TIME_TO_DAY_MASK);
    flags |= ALARM_TIME_FROM_DAY_MASK & (from_day << 3);
    flags |= ALARM_TIME_TO_DAY_MASK & to_day;

    set_alarm_time_var ((ALARM_TIME_VARIABLE) idx, (uint_fast16_t) (hour * 60 + minute), flags);

    http_json_ok ();

    return 0;
}

static int
http_api_overlay_set ()
{
    int     idx;
    int     n_overlays = (int) get_numvar (OVERLAY_N_OVERLAYS_NUM_VAR);
    int     type;
    int     date_code;
    int     interval = 5;
    int     duration = 5;
    int     month = 0;
    int     day = 0;
    int     days = 1;
    char *  text;

    if (! http_get_int_param ("idx", &idx))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "idx missing or not numeric");
        return 0;
    }

    /* Bisher kam hier {"ok":true} heraus, obwohl nichts geschrieben wurde: ein idx
     * ausserhalb des Bereichs oder jenseits der belegten Overlays fiel stumm durch,
     * und die Oberflaeche zeigte danach ein Overlay, das im Geraet nicht existiert
     * (Fehlerklasse von L30). idx == n_overlays haengt ein neues Overlay an.
     */
    if (idx < 0 || idx >= MAX_OVERLAYS || idx > n_overlays)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "overlay index out of range or not in use");
        return 0;
    }

    /* Alles wird VOR der ersten Zuweisung geprueft. Sonst waechst bei einem Anhaenge-
     * Aufruf erst n_overlays, und die Abweisung danach hinterlaesst ein leeres Overlay
     * in der Liste.
     *
     * type und date_code haben keinen sinnvollen Rueckfallwert: Fehlen sie, wurde das
     * Overlay bisher still auf Typ 0 und Datumscode 0 umgestellt (L29).
     */
    if (! http_get_int_param ("type", &type) || ! http_get_int_param ("date_code", &date_code))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "type and date_code required");
        return 0;
    }

    /* Definiert sind 0..10, angenommen wurde bisher alles bis 255 (L73). */
    if (type < OVERLAY_TYPE_NONE || type > OVERLAY_TYPE_TEMPERATURE_DIGITS)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "type out of range (0..10)");
        return 0;
    }

    /* date_code hatte gar keine Bereichspruefung. 300 ging als "12c" an den STM, der
     * davon die ersten zwei Ziffern las und "invalid date_code: 18" meldete (L66).
     */
    if (date_code < OVERLAY_DATE_CODE_NONE || date_code > OVERLAY_DATE_CODE_ADVENT4)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "date_code out of range (0..6)");
        return 0;
    }

    /* interval, duration, month, day und days behalten ihre Rueckfallwerte, wenn sie
     * fehlen - das ist Absicht (L29). Ein VORHANDENER Wert muss aber in den Bereich
     * passen, den der STM mit fester Breite liest (L66).
     *
     * Geprueft wird jetzt gegen den ECHTEN Bereich, und jede Abweisung nennt ihn:
     * interval 0 wurde still zu 5, days 0 still zu 1, und duration trug einen
     * Doppelvertrag - erst auf 0..255 geprueft, danach auf 5..9 geklemmt. In allen drei
     * Faellen meldete das Geraet {"ok":true} auf einen Wert, der nie angekommen ist
     * (L186).
     */
    if (! http_get_opt_int_param ("interval", &interval, 1, 255))
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "interval out of range (1..255)");
        return 0;
    }

    if (! http_get_opt_int_param ("duration", &duration, 5, 9))
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "duration out of range (5..9)");
        return 0;
    }

    if (! http_get_opt_int_param ("month", &month, 0, 12))
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "month out of range (0..12)");
        return 0;
    }

    if (! http_get_opt_int_param ("day", &day, 0, 31))
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "day out of range (0..31)");
        return 0;
    }

    if (! http_get_opt_int_param ("days", &days, 1, 255))
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "days out of range (1..255)");
        return 0;
    }

    /* Eine Teilangabe des Startdatums wurde bisher stillschweigend verworfen: Wer month
     * ohne day schickte, bekam {"ok":true} und ein Overlay ohne Startdatum. Die Pruefung
     * steht hier oben, weil weiter unten bereits n_overlays erhoeht wird - eine Abweisung
     * danach hinterliesse ein leeres Overlay in der Liste.
     */
    if ((month < 1) != (day < 1))
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "month (1..12) and day (1..31) must both be set or both be 0");
        return 0;
    }

    text = http_get_param ("value");

    /* Overlay-Text, ESP (L319): ein Text ueber OVERLAY_MAX_TEXT_LEN Byte wurde still gekuerzt
     * und mit {"ok":true} quittiert. Jetzt Abweisung mit Kennung 2 ueber dieselbe Byte-Regel wie
     * die Stringsetter (C22) - und VOR der ersten Zuweisung, damit nichts geschrieben wird.
     */
    if (! http_check_strvar_len ("value", text, OVERLAY_MAX_TEXT_LEN))
    {
        return 0;
    }

    if (idx == n_overlays)
    {
        n_overlays++;
        set_numvar (OVERLAY_N_OVERLAYS_NUM_VAR, n_overlays);
    }

    if (http_get_on_off_value ("active", 0))
    {
        overlays[idx].flags |= OVERLAY_FLAG_ACTIVE;
    }
    else
    {
        overlays[idx].flags &= ~OVERLAY_FLAG_ACTIVE;
    }

    overlays[idx].type      = type;
    overlays[idx].interval  = interval;
    overlays[idx].duration  = duration;
    overlays[idx].date_code = date_code;
    overlays[idx].days      = days;

    if (month < 1 || day < 1)
    {
        overlays[idx].date_start = 0;
    }
    else
    {
        overlays[idx].date_start = (month << 8) | day;
    }

    /* Overlay-Texte stehen ebenfalls in der settings_xml und kennen dieselbe
     * Byte-gegen-Zeichen-Grenze wie die Stringsetter (L46). Das frueher hier stehende
     * "if (text)" war die POSITIVFORM einer toten NULL-Pruefung (C20/L199) -- ein
     * fehlender Parameter liefert den leeren String, und der wurde ohnehin kopiert.
     * Die Laenge ist oben geprueft; utf8_copy_truncated() bleibt nur als letztes Netz
     * stehen und kuerzt hier nichts mehr - wie set_strvar() hinter den Stringsettern.
     */
    utf8_copy_truncated (overlays[idx].text, text, OVERLAY_MAX_TEXT_LEN);

    set_overlay_var (idx);

    http_json_ok ();

    return 0;
}

/* idx war optional und wurde per atoi zu 0, die Antwort war trotzdem {"ok":true}:
 * overlay_display ohne idx zeigte das erste Overlay, overlay_delete ohne idx hat es
 * geloescht, und ein unsinniger idx meldete Erfolg und tat nichts (L70).
 */
static uint_fast8_t
http_get_overlay_idx_param (int * idxp)
{
    int n_overlays = (int) http_n_overlays_for_read ();                         // S.5: Lesen begrenzen

    if (! http_get_int_param ("idx", idxp))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "idx missing or not numeric");
        return 0;
    }

    if (*idxp < 0 || *idxp >= n_overlays)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "overlay index out of range or not in use");
        return 0;
    }

    return 1;
}

static int
http_api_overlay_display ()
{
    int idx;

    if (! http_get_overlay_idx_param (&idx))
    {
        return 0;
    }

    set_numvar (DISPLAY_OVERLAY_NUM_VAR, idx);

    http_json_ok ();

    return 0;
}

static int
http_api_overlay_delete ()
{
    int idx;
    int n_overlays;
    int i;

    if (! http_get_overlay_idx_param (&idx))
    {
        return 0;
    }

    n_overlays = (int) get_numvar (OVERLAY_N_OVERLAYS_NUM_VAR);

    /* S.5: Schreiben abweisen. Mit mehr als MAX_OVERLAYS vom STM liefe das Nachruecken hinter
     * overlays[] und memset() schriebe dahinter. Der idx selbst ist oben schon begrenzt.
     */
    if (n_overlays > MAX_OVERLAYS)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "overlay count reported by STM32 out of range");
        return 0;
    }

    for (i = idx; i < n_overlays - 1; i++)
    {
        overlays[i] = overlays[i + 1];
        set_overlay_var (i);
    }

    memset (&overlays[n_overlays - 1], 0, sizeof (OVERLAY));
    set_overlay_var (n_overlays - 1);
    set_numvar (OVERLAY_N_OVERLAYS_NUM_VAR, n_overlays - 1);

    http_json_ok ();

    return 0;
}

/* Beide Timer-Endpunkte unterscheiden sich nur in is_ambilight. Vorher war jeder
 * Parameter optional und wurde per atoi zu 0 (L70/L71):
 *   - ohne idx ueberschrieb der Aufruf still den ersten Timer
 *   - ohne hour/minute entstand ein aktiver Timer auf 00:00
 *   - hour=99&minute=99 ergab minutes=6039; ein Tag hat 1440, der Timer konnte nie
 *     ausloesen und stand trotzdem als aktiv in der Liste
 *   - from=9 wurde ungeprueft auf 3 Bit maskiert und damit zu Montag
 */
static int
http_api_timer_set_common (uint_fast8_t is_ambilight)
{
    int             idx;
    int             from_day;
    int             to_day;
    int             hour;
    int             minute;
    uint_fast8_t    flags = 0;

    if (! http_get_int_param ("idx", &idx) ||
        ! http_get_int_param ("from", &from_day) ||
        ! http_get_int_param ("to", &to_day) ||
        ! http_get_int_param ("hour", &hour) ||
        ! http_get_int_param ("minute", &minute))
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "idx, from, to, hour and minute required");
        return 0;
    }

    if (idx < 0 || idx >= MAX_NIGHT_TIME_VARIABLES)
    {
        http_json_error_range ("idx", 0, MAX_NIGHT_TIME_VARIABLES - 1);
        return 0;
    }

    if (from_day < 0 || from_day > HTTP_MAX_WEEKDAY || to_day < 0 || to_day > HTTP_MAX_WEEKDAY)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "from and to out of range (0..6)");
        return 0;
    }

    if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
    {
        http_json_error (HTTP_API_ERROR_OUT_OF_RANGE, "hour out of range (0..23) or minute out of range (0..59)");
        return 0;
    }

    if (http_get_on_off_value ("active", 0))
    {
        flags |= NIGHT_TIME_FLAG_ACTIVE;
    }

    if (http_get_on_off_value ("switch_on", 0))
    {
        flags |= NIGHT_TIME_FLAG_SWITCH_ON;
    }

    flags |= NIGHT_TIME_FROM_DAY_MASK & (from_day << 3);
    flags |= NIGHT_TIME_TO_DAY_MASK & to_day;

    set_night_time_var (is_ambilight, (NIGHT_TIME_VARIABLE) idx, (uint_fast16_t) (hour * 60 + minute), flags);

    http_json_ok ();

    return 0;
}

static int
http_api_timer_set ()
{
    return http_api_timer_set_common (0);
}

static int
http_api_ambilight_timer_set ()
{
    return http_api_timer_set_common (1);
}

static int
http_api_overlay_icons ()
{
    static const char * const icon_candidates[] = { "wc24h-icon.txt", "wc12h-icon.txt", "uc-icon.txt" };
    const char *  fname = (const char *) 0;

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("["));

    LittleFS.begin ();
    fname = http_find_existing_filename (http_get_configured_icon_filename (), icon_candidates, sizeof (icon_candidates) / sizeof (icon_candidates[0]));

    if (fname)
    {
        File fp = LittleFS.open (fname, "r");

        if (fp)
        {
            int   ch = fp.read ();
            int   first = 1;
            char  icon_name[32 + 1];

            while (ch == '*')
            {
                int i = 0;

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

                if (! first)
                {
                    http_send (FS(","));
                }

                http_send (FS("\""));
                http_send_json_escaped (icon_name);
                http_send (FS("\""));
                first = 0;

                while ((ch = fp.read()) != '*' && ch >= 0)
                {
                    ;
                }
            }

            fp.close ();
        }
    }

    LittleFS.end ();

    http_send (FS("]"));
    http_flush ();

    return 0;
}

static int
http_api_fs_info ()
{
    FSInfo fsinfo;

    LittleFS.begin ();

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));

    if (LittleFS.info (fsinfo))
    {
        char buf[16];

        http_send (FS("{\"ok\":true"));

        sprintf (buf, "%d", fsinfo.totalBytes);
        http_send (FS(",\"total\":"));
        http_send (buf);

        sprintf (buf, "%d", fsinfo.usedBytes);
        http_send (FS(",\"used\":"));
        http_send (buf);

        sprintf (buf, "%d", fsinfo.blockSize);
        http_send (FS(",\"block_size\":"));
        http_send (buf);

        sprintf (buf, "%d", fsinfo.pageSize);
        http_send (FS(",\"page_size\":"));
        http_send (buf);

        sprintf (buf, "%d", fsinfo.maxOpenFiles);
        http_send (FS(",\"max_open_files\":"));
        http_send (buf);

        sprintf (buf, "%d", fsinfo.maxPathLength);
        http_send (FS(",\"max_path_length\":"));
        http_send (buf);

        http_send (FS("}"));
    }
    else
    {
        http_send (FS("{\"ok\":false}"));
    }

    LittleFS.end ();
    http_flush ();

    return 0;
}

static int
http_api_fs_list ()
{
    Dir dir;
    int first = 1;

    LittleFS.begin ();
    dir = LittleFS.openDir ("");

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"files\":["));

    while (dir.next ())
    {
        File f = dir.openFile ("r");
        char sizebuf[16];

        if (! first)
        {
            http_send (FS(","));
        }

        sprintf (sizebuf, "%d", f ? f.size () : 0);
        http_send (FS("{\"name\":\""));
        http_send_json_escaped (dir.fileName ().c_str ());
        http_send (FS("\",\"size\":"));
        http_send (sizebuf);
        http_send (FS("}"));

        if (f)
        {
            f.close ();
        }

        first = 0;
    }

    LittleFS.end ();

    http_send (FS("]}"));
    http_flush ();

    return 0;
}

/* Drei Faelle statt zwei -- Befund L187 / C16. Bis hierher gingen die Kopfzeilen hinaus,
 * BEVOR die Datei geoeffnet wurde. Scheiterte LittleFS.open(), folgte gar nichts mehr: Die
 * Antwort war ein leerer Rumpf mit Content-Type text/plain, und die PWA konnte "Datei leer"
 * nicht von "Datei fehlt" unterscheiden. Eine leere Datei ist hier kein theoretischer Fall,
 * sondern genau der Weisschirm-Fehlerfall der Architektur-Invariante.
 *
 * Unterschieden wird am Content-Type, nicht am Rumpfanfang:
 *   Parameter fehlt oder leer  -> application/json, error 1
 *   Datei existiert nicht      -> application/json, error 6
 *   existiert, oeffnet nicht   -> application/json, error 7 (Review F.6; bis dahin 6)
 *   Datei existiert, Groesse 0 -> application/json, error 8 (C27; bis ESP 3.2.25 text/plain, leer)
 *   Datei existiert mit Inhalt -> text/plain mit Inhalt -- unveraendert
 * Eine Pruefung auf {"ok":false am Rumpfanfang waere falsch, sobald eine angezeigte Datei
 * genau so beginnt.
 *
 * http_fs_file_exists_and_nonempty() ist hier ABSICHTLICH nicht benutzt: Der Helfer prueft
 * zusaetzlich auf Groesse > 0 und wuerde die leere Datei als "fehlt" melden -- genau die
 * Verwechslung, die dieser Umbau abschafft.
 *
 * Ein begin(), ein end() auf jedem Pfad, auch auf den Fehlerpfaden (C9b / L129).
 * Der Dateiname wandert NICHT in den detail-Text; ein fester Text spart die Frage nach
 * seiner Maskierung ganz.
 */
static int
http_api_fs_show ()
{
    char * fname = http_get_param ("filename");

    if (! *fname)                                           // nie NULL, siehe Vertrag bei http_get_param (C20/L199)
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "filename required");
        return 0;
    }

    LittleFS.begin ();

    if (! LittleFS.exists (fname))
    {
        LittleFS.end ();
        http_json_error (HTTP_API_ERROR_NOT_FOUND, "file not found");
        return 0;
    }

    File fp = LittleFS.open (fname, "r");

    if (! fp)                                                               // existiert, laesst sich aber nicht oeffnen
    {
        LittleFS.end ();
        http_json_error (HTTP_API_ERROR_ACTION_FAILED, "open failed");      // Review F.6: nicht "gibt es nicht" (6), sondern 7
        return 0;
    }

    if (fp.size () == 0)                                                    // C27: leer ist ein eigener Zustand, kein 200 OK ohne Rumpf
    {
        fp.close ();
        LittleFS.end ();
        http_json_error (HTTP_API_ERROR_FILE_EMPTY, "file empty");
        return 0;
    }

    // Ab hier steht fest, dass Inhalt folgt -- erst jetzt gehen die Kopfzeilen hinaus.
    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nCache-Control: no-cache\r\n\r\n"));

    while (fp.available ())
    {
        char b[64];
        int n = fp.readBytes (b, sizeof (b));

        if (n > 0)
        {
            for (int i = 0; i < n; i++)
            {
                char cbuf[2];
                cbuf[0] = b[i];
                cbuf[1] = '\0';
                http_send (cbuf);
            }
        }
        else
        {
            break;                                                          // C34/L309: Lesefehler, sonst dreht available() endlos bis zum Soft-WDT
        }
    }

    fp.close ();
    LittleFS.end ();

    http_flush ();

    return 0;
}

/* C20/L199: Hier kam {"ok":true} heraus, egal was passiert ist -- bei fehlendem Dateinamen
 * (dann lief der Block gar nicht), bei nicht vorhandener Datei und bei einem Fehlschlag von
 * LittleFS.remove(), dessen Rueckgabewert verworfen wurde. Die Oberflaeche meldete
 * "geloescht" und die Datei lag noch da.
 *
 * Ein begin(), ein end() auf jedem Pfad, auch auf den Fehlerpfaden (C9b/L129). Der Dateiname
 * wandert NICHT in den detail-Text -- ein fester Text spart die Frage nach seiner Maskierung
 * ganz, dieselbe Entscheidung wie bei http_api_fs_show().
 */
static int
http_api_fs_remove ()
{
    char * fname = http_get_param ("filename");
    bool   removed;

    if (! *fname)                                           // nie NULL, siehe Vertrag bei http_get_param (C20/L199)
    {
        http_json_error (HTTP_API_ERROR_MISSING_VALUE, "filename required");
        return 0;
    }

    LittleFS.begin ();

    if (! LittleFS.exists (fname))
    {
        LittleFS.end ();
        http_json_error (HTTP_API_ERROR_NOT_FOUND, "file not found");
        return 0;
    }

    removed = LittleFS.remove (fname);
    LittleFS.end ();

    if (! removed)
    {
        http_json_error (HTTP_API_ERROR_ACTION_FAILED, "remove failed");
        return 0;
    }

    http_json_ok ();

    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * Zwischenspeicher und Sperrfenster fuer den Update-Server (BEFUNDE.md L161, L169)
 *
 * /api/update_status machte je Aufruf SIEBEN ausgehende HTTP-Abrufe, und die PWA fragt
 * den Endpunkt zyklisch ab. Jeder Abruf kann DNS bis 5000 ms, Verbindungsaufbau bis
 * 5000 ms und Warten auf das erste Byte bis 5000 ms kosten; waehrend der ganzen Zeit
 * laeuft loop () nicht, wird keine Verbindung angenommen und keine wartende bedient.
 * Auf genau diesem Pfad lag der Absturz vom 04.10.2026 mit bekanntem Ausloeser (L169).
 *
 * Zwei Massnahmen, beide OHNE Heap - frei sind rund 5'400 bis 6'400 Byte (L134):
 *
 * 1. ZWISCHENSPEICHER fuer die fuenf billigen Serverwerte (drei Versionszeilen, zwei
 *    Existenzpruefungen). Sie aendern sich nicht im Sekundentakt. 54 Byte im statischen
 *    Bereich; ein warmer Aufruf macht statt sieben nur noch zwei Abrufe.
 *    Die Release Notes und wc-list.txt werden BEWUSST NICHT zwischengespeichert: beide
 *    werden unmittelbar in die laufende Antwort gestroemt, ein Puffer dafuer waere bis
 *    zu 3072 Byte gross und wuerde den freien Speicher dauerhaft um mehr als die
 *    Haelfte dessen verringern, was L147 gerade erst freigeraeumt hat.
 *
 * 2. SPERRFENSTER. Faellt ein Abruf langsam aus - also nicht mit "404 sofort", sondern
 *    nach einer Zeitgrenze -, gilt der Server fuer UPDATE_SERVER_DOWN_MS als nicht
 *    erreichbar und die uebrigen Abrufe desselben und der folgenden Aufrufe entfallen.
 *    Ohne das kostete ein nicht erreichbarer Update-Server bis zu sieben Zeitgrenzen in
 *    EINEM Request. Der erste erfolgreiche Abruf hebt die Sperre sofort auf.
 *
 * GUELTIGKEIT: Der Zwischenspeicher haengt an Host, Pfad UND hardware_configuration -
 * die Namen der Icon- und Wetterdatei werden daraus gebildet. Statt die Setter
 * (/api/update_host_set, /api/update_path_set, die Legacy-Formulare) einzeln zu
 * benachrichtigen, wird eine Pruefsumme ueber diese drei Werte mitgefuehrt: dann kann
 * keine Aenderung uebersehen werden, auch keine kuenftige.
 *
 * ERZWUNGENE AUFFRISCHUNG: "?refresh=1" an /api/update_status umgeht den
 * Zwischenspeicher. Die PWA sendet den Parameter heute nicht; die Antwortform aendert
 * sich dadurch nicht, der API-Vertrag bleibt also unberuehrt.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define UPDATE_INFO_CACHE_MS                    60000UL         // Gueltigkeitsdauer der zwischengespeicherten Serverwerte
#define UPDATE_SERVER_DOWN_MS                   30000UL         // Sperrfenster nach einem langsamen Fehlschlag
#define UPDATE_SERVER_SLOW_FAIL_MS               1000UL         // ab hier gilt ein Fehlschlag als "Server antwortet nicht"

static char          update_cache_esp_version[16];
static char          update_cache_app_version[16];
static char          update_cache_wc_version[16];
static unsigned long update_cache_millis        = 0;
static unsigned long update_cache_window        = UPDATE_INFO_CACHE_MS;
static uint16_t      update_cache_source_sum    = 0;
static uint_fast8_t  update_cache_valid         = 0;
static uint_fast8_t  update_cache_assets        = 0;
static uint16_t      update_cache_hits          = 0;            // Diagnose, steht in /api/device_ready

static unsigned long update_server_down_millis  = 0;
static uint_fast8_t  update_server_down         = 0;
static uint16_t      update_server_down_count   = 0;            // Diagnose, steht in /api/device_ready

static uint16_t
update_source_checksum (const char * host, const char * path)
{
    uint16_t sum = (uint16_t) hardware_configuration;

    while (*host)
    {
        sum = (uint16_t) (sum * 31u + (unsigned char) *host++);
    }

    sum = (uint16_t) (sum * 31u + '/');

    while (*path)
    {
        sum = (uint16_t) (sum * 31u + (unsigned char) *path++);
    }

    return sum;
}

static uint_fast8_t
update_server_blocked (void)
{
    if (! update_server_down)
    {
        return 0;
    }

    if ((millis () - update_server_down_millis) >= UPDATE_SERVER_DOWN_MS)
    {
        update_server_down = 0;                                 // Fenster abgelaufen, naechster Versuch ist frei
        return 0;
    }

    return 1;
}

static void
update_server_note (int len, unsigned long elapsed)
{
    if (len > 0)
    {
        update_server_down = 0;                                 // Server antwortet wieder
    }
    else if (elapsed >= UPDATE_SERVER_SLOW_FAIL_MS)
    {
        /* Nur LANGSAME Fehlschlaege sperren. Ein sofortiges 404 kostet nichts und darf
         * die uebrigen Abrufe nicht verhindern - sonst verschwaende ein einzelner
         * fehlender Dateiname auf dem Server die ganze Statusabfrage.
         */
        update_server_down        = 1;
        update_server_down_millis = millis ();

        if (update_server_down_count < 0xFFFF)
        {
            update_server_down_count++;
        }
    }
}

static int
update_server_open (const char * host, const char * path, const char * filename)
{
    unsigned long   start_millis;
    int             len;

    if (update_server_blocked ())
    {
        return -1;
    }

    start_millis = millis ();
    len          = httpclient (host, path, filename);
    update_server_note (len, millis () - start_millis);

    return len;
}

static int
update_server_fetch_line (const char * host, const char * path, const char * filename, char * buffer, size_t buffer_len)
{
    unsigned long   start_millis;
    int             len;

    if (buffer_len > 0)
    {
        buffer[0] = '\0';
    }

    if (update_server_blocked ())
    {
        return -1;
    }

    start_millis = millis ();
    len          = http_fetch_remote_line (host, path, filename, buffer, buffer_len);
    update_server_note (len, millis () - start_millis);

    return len;
}

static int
http_api_update_status ()
{
    STR_VAR * sv;
    char * update_host;
    char * update_path;
    uint32_t flashsize;
    char new_esp_version[16];
    char new_app_version[16];
    char new_wc_version[16];
    char stm32_default_filename[MAX_UPDATE_FILENAME_LEN];
    const char * filter = (const char *) NULL;
    uint_fast8_t assets_available = 0;
    const char * fname_icon = (const char *) 0;
    const char * fname_weather = (const char *) 0;
    char * refresh;
    uint16_t source_sum;
    uint_fast8_t from_cache = 0;
    unsigned long cache_window = UPDATE_INFO_CACHE_MS;
    int len;
    int first = 1;

    sv = get_strvar (UPDATE_HOST_VAR);
    update_host = sv->str;

    if (! update_host[0])
    {
        update_host = (char *) DEFAULT_UPDATE_HOST;
    }

    sv = get_strvar (UPDATE_PATH_VAR);
    update_path = sv->str;

    if (! update_path[0])
    {
        update_path = (char *) DEFAULT_UPDATE_PATH;
    }

    flashsize = ESP.getFlashChipRealSize ();
    http_build_stm32_default_filename (stm32_default_filename, sizeof (stm32_default_filename), &filter);

    source_sum = update_source_checksum (update_host, update_path);
    refresh    = http_get_param ("refresh");

    /* Zwischenspeicher nur verwenden, wenn ALLE vier Bedingungen stimmen: gefuellt,
     * gleiche Quelle (Host, Pfad, Hardware), nicht aelter als update_cache_window und
     * keine erzwungene Auffrischung. Die Differenzbildung auf millis () ist
     * ueberlaufsicher; update_cache_valid deckt den Startfall ab, in dem
     * update_cache_millis noch 0 ist.
     */
    if (update_cache_valid
        && update_cache_source_sum == source_sum
        && (millis () - update_cache_millis) < update_cache_window
        && ! (refresh[0] && refresh[0] != '0'))
    {
        strcpy (new_esp_version, update_cache_esp_version);
        strcpy (new_app_version, update_cache_app_version);
        strcpy (new_wc_version, update_cache_wc_version);
        assets_available = update_cache_assets;
        from_cache       = 1;

        if (update_cache_hits < 0xFFFF)
        {
            update_cache_hits++;
        }
    }
    else
    {
        int esp_len = update_server_fetch_line (update_host, update_path, ESP_WORDCLOCK_TXT, new_esp_version, sizeof (new_esp_version));
        int app_len = update_server_fetch_line (update_host, update_path, APP_VERSION_TXT, new_app_version, sizeof (new_app_version));
        int wc_len  = update_server_fetch_line (update_host, update_path, WC_TXT, new_wc_version, sizeof (new_wc_version));

        /* Ein Fehlschlag wird KUERZER zwischengespeichert als ein Erfolg. Gar nicht
         * ablegen hiesse, dass ein nicht erreichbarer Server bei jedem PWA-Zyklus erneut
         * in die Zeitgrenzen laeuft; voll ablegen hiesse, dass er nach seiner Rueckkehr
         * eine Minute lang weiter als tot gilt. Das kuerzere Fenster entspricht genau dem
         * Sperrfenster: Sobald dieses abgelaufen ist, darf wieder gefragt werden.
         */
        cache_window = (esp_len > 0 || app_len > 0 || wc_len > 0) ? UPDATE_INFO_CACHE_MS : UPDATE_SERVER_DOWN_MS;
    }

    if (hardware_configuration != 0xFFFF)
    {
        switch (hardware_configuration & HW_WC_MASK)
        {
            case HW_WC_24H:
                fname_icon = "wc24h-icon.txt";
                fname_weather = "wc24h-weather.txt";
                break;
            case HW_WC_12H:
                fname_icon = "wc12h-icon.txt";
                fname_weather = "wc12h-weather.txt";
                break;
            case HW_UCLOCK:
                fname_icon = "uc-icon.txt";
                fname_weather = "uc-weather.txt";
                break;
        }
    }

    if (! from_cache)
    {
        if (fname_icon && fname_weather)
        {
            int len_icon = update_server_open (update_host, update_path, fname_icon);

            if (len_icon > 0)
            {
                httpclient_stop ();

                int len_weather = update_server_open (update_host, update_path, fname_weather);

                if (len_weather > 0)
                {
                    assets_available = 1;
                    httpclient_stop ();
                }
            }
        }

        /* Erst jetzt ablegen - vorher steht assets_available noch nicht fest. */
        strcpy (update_cache_esp_version, new_esp_version);
        strcpy (update_cache_app_version, new_app_version);
        strcpy (update_cache_wc_version, new_wc_version);
        update_cache_assets     = assets_available;
        update_cache_source_sum = source_sum;
        update_cache_window     = cache_window;
        update_cache_millis     = millis ();
        update_cache_valid      = 1;
    }

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"flash_size\":"));

    char buf[16];
    sprintf (buf, "%d", flashsize);
    http_send (buf);

    http_json_send_bool_field (FS("can_update"), flashsize >= 4194304UL);
    http_json_send_bool_field (FS("local_update_supported"), flashsize >= 1048576UL);
    http_json_send_string_field (FS("local_update_message"), http_get_local_update_message (flashsize));
    http_json_send_bool_field (FS("assets_available"), assets_available);
    /* Zusaetzliches Feld, rein additiv: true heisst, die fuenf Serverwerte stammen aus
     * dem Zwischenspeicher und sind bis zu UPDATE_INFO_CACHE_MS alt. "?refresh=1"
     * erzwingt frische Werte.
     */
    http_json_send_bool_field (FS("update_info_cached"), from_cache);
    http_json_send_bool_field (FS("app_bundle_available"), 0);
    http_json_send_bool_field (FS("device_ready_api_supported"), 1);
    http_json_send_bool_field (FS("reconnect_probe_api_supported"), 1);
    http_json_send_bool_field (FS("settings_api_supported"), 1);
    http_json_send_bool_field (FS("display_power_api_supported"), 1);
    http_json_send_bool_field (FS("ambilight_power_api_supported"), 1);
    http_json_send_bool_field (FS("power_status_api_supported"), 1);
    http_json_send_bool_field (FS("update_download_assets_api_supported"), 1);
    http_json_send_bool_field (FS("update_download_app_bundle_api_supported"), 0);
    http_json_send_bool_field (FS("update_download_table_api_supported"), 1);
    http_json_send_bool_field (FS("app_file_upload_api_supported"), 1);
    http_json_send_bool_field (FS("app_bundle_upload_api_supported"), 0);
    http_json_send_bool_field (FS("fs_target_upload_api_supported"), 1);
    http_json_send_bool_field (FS("local_stm32_upload_api_supported"), 1);
    http_json_send_bool_field (FS("local_esp_update_api_supported"), 1);
    http_json_send_remote_update_support_fields ();

    http_json_send_string_field (FS("device_ready_url"), "/api/device_ready");
    http_json_send_string_field (FS("reconnect_probe_url"), "/api/reconnect_probe");
    http_json_send_remote_update_url_fields ();
    http_json_send_string_field (FS("update_status_url"), "/api/update_status");
    http_json_send_string_field (FS("settings_url"), "/api/settings_xml");
    http_json_send_string_field (FS("display_power_url"), "/api/display_power");
    http_json_send_string_field (FS("display_power_set_url"), "/api/display_power_set");
    http_json_send_string_field (FS("display_test_url"), "/api/test_display");
    http_json_send_string_field (FS("display_brightness_set_url"), "/api/display_brightness_set");
    http_json_send_string_field (FS("display_it_is_set_url"), "/api/display_it_is_set");
    http_json_send_string_field (FS("display_mode_set_url"), "/api/display_mode_set");
    http_json_send_string_field (FS("display_use_rgbw_set_url"), "/api/display_use_rgbw_set");
    http_json_send_string_field (FS("ticker_set_url"), "/api/ticker_set");
    http_json_send_string_field (FS("date_ticker_format_set_url"), "/api/date_ticker_format_set");
    http_json_send_string_field (FS("ticker_deceleration_set_url"), "/api/ticker_deceleration_set");
    http_json_send_string_field (FS("ambilight_power_url"), "/api/ambilight_power");
    http_json_send_string_field (FS("ambilight_power_set_url"), "/api/ambilight_power_set");
    http_json_send_string_field (FS("ambilight_online_set_url"), "/api/ambilight_online_set");
    http_json_send_string_field (FS("power_status_url"), "/api/power_status");
    http_json_send_string_field (FS("maintenance_reset_stm32_url"), "/api/maintenance_reset_stm32");
    http_json_send_string_field (FS("maintenance_reset_eeprom_url"), "/api/maintenance_reset_eeprom");
    http_json_send_string_field (FS("maintenance_format_fs_url"), "/api/maintenance_format_fs");
    http_json_send_string_field (FS("auto_brightness_set_url"), "/api/auto_brightness_set");
    http_json_send_string_field (FS("network_timeserver_set_url"), "/api/network_timeserver_set");
    http_json_send_string_field (FS("network_client_set_url"), "/api/network_client_set");
    http_json_send_string_field (FS("network_ap_set_url"), "/api/network_ap_set");
    http_json_send_string_field (FS("network_timezone_set_url"), "/api/network_timezone_set");
    http_json_send_string_field (FS("network_summertime_set_url"), "/api/network_summertime_set");
    http_json_send_string_field (FS("update_host_set_url"), "/api/update_host_set");
    http_json_send_string_field (FS("update_path_set_url"), "/api/update_path_set");
    http_json_send_string_field (FS("datetime_set_url"), "/api/datetime_set");
    http_json_send_string_field (FS("weather_appid_set_url"), "/api/weather_appid_set");
    http_json_send_string_field (FS("weather_city_set_url"), "/api/weather_city_set");
    http_json_send_string_field (FS("weather_coordinates_set_url"), "/api/weather_coordinates_set");
    http_json_send_string_field (FS("weather_get_now_url"), "/api/weather_get_now");
    http_json_send_string_field (FS("weather_get_forecast_url"), "/api/weather_get_forecast");
    http_json_send_string_field (FS("learn_ir_url"), "/api/learn_ir");
    http_json_send_string_field (FS("network_get_time_url"), "/api/network_get_time");
    http_json_send_string_field (FS("network_wps_url"), "/api/network_wps");
    http_json_send_string_field (FS("temperature_display_url"), "/api/temperature_display");
    http_json_send_string_field (FS("temperature_rtc_correction_set_url"), "/api/temperature_rtc_correction_set");
    http_json_send_string_field (FS("temperature_ds18xx_correction_set_url"), "/api/temperature_ds18xx_correction_set");
    http_json_send_string_field (FS("ldr_min_set_url"), "/api/ldr_min_set");
    http_json_send_string_field (FS("ldr_max_set_url"), "/api/ldr_max_set");
    http_json_send_string_field (FS("ldr_min_value_set_url"), "/api/ldr_min_value_set");
    http_json_send_string_field (FS("ldr_max_value_set_url"), "/api/ldr_max_value_set");
    http_json_send_string_field (FS("animation_mode_set_url"), "/api/animation_mode_set");
    http_json_send_string_field (FS("color_animation_mode_set_url"), "/api/color_animation_mode_set");
    http_json_send_string_field (FS("sync_ambilight_set_url"), "/api/sync_ambilight_set");
    http_json_send_string_field (FS("sync_markers_set_url"), "/api/sync_markers_set");
    http_json_send_string_field (FS("fade_clock_seconds_set_url"), "/api/fade_clock_seconds_set");
    http_json_send_string_field (FS("ambilight_markers_set_url"), "/api/ambilight_markers_set");
    http_json_send_string_field (FS("ambilight_brightness_set_url"), "/api/ambilight_brightness_set");
    http_json_send_string_field (FS("ambilight_mode_set_url"), "/api/ambilight_mode_set");
    http_json_send_string_field (FS("ambilight_leds_set_url"), "/api/ambilight_leds_set");
    http_json_send_string_field (FS("ambilight_offset_set_url"), "/api/ambilight_offset_set");
    http_json_send_string_field (FS("display_color_set_url"), "/api/display_color_set");
    http_json_send_string_field (FS("ambilight_color_set_url"), "/api/ambilight_color_set");
    http_json_send_string_field (FS("marker_color_set_url"), "/api/marker_color_set");
    http_json_send_string_field (FS("dfplayer_volume_set_url"), "/api/dfplayer_volume_set");
    http_json_send_string_field (FS("dfplayer_mode_set_url"), "/api/dfplayer_mode_set");
    http_json_send_string_field (FS("dfplayer_bell_flags_set_url"), "/api/dfplayer_bell_flags_set");
    http_json_send_string_field (FS("dfplayer_speak_cycle_set_url"), "/api/dfplayer_speak_cycle_set");
    http_json_send_string_field (FS("dfplayer_silence_start_set_url"), "/api/dfplayer_silence_start_set");
    http_json_send_string_field (FS("dfplayer_silence_stop_set_url"), "/api/dfplayer_silence_stop_set");
    http_json_send_string_field (FS("dfplayer_play_url"), "/api/dfplayer_play");
    http_json_send_string_field (FS("dfplayer_alarm_set_url"), "/api/dfplayer_alarm_set");
    http_json_send_string_field (FS("overlay_set_url"), "/api/overlay_set");
    http_json_send_string_field (FS("overlay_display_url"), "/api/overlay_display");
    http_json_send_string_field (FS("overlay_delete_url"), "/api/overlay_delete");
    http_json_send_string_field (FS("timer_set_url"), "/api/timer_set");
    http_json_send_string_field (FS("ambilight_timer_set_url"), "/api/ambilight_timer_set");
    http_json_send_string_field (FS("update_download_assets_url"), "/api/update_download_assets");
    http_json_send_string_field (FS("update_download_app_bundle_url"), "");
    http_json_send_string_field (FS("update_download_table_base_url"), "/api/update_download_table?filename=");
    http_json_send_string_field (FS("app_file_upload_url"), "/api/app_file_upload");
    http_json_send_string_field (FS("app_bundle_upload_url"), "");
    http_json_send_string_field (FS("fs_info_url"), "/api/fs_info");
    http_json_send_string_field (FS("fs_list_url"), "/api/fs_list");
    http_json_send_string_field (FS("eeprom_settings_url"), "/api/eeprom_settings");
    http_json_send_string_field (FS("update_table_files_url"), "/api/update_table_files");
    http_json_send_string_field (FS("fs_upload_icon_url"), "/api/fs_upload_icon");
    http_json_send_string_field (FS("fs_upload_weather_url"), "/api/fs_upload_weather");
    http_json_send_string_field (FS("fs_upload_tables_url"), "/api/fs_upload_tables");
    http_json_send_string_field (FS("fs_upload_display_url"), "/api/fs_upload_display");
    http_json_send_string_field (FS("fs_upload_txt_accept"), ".txt,text/plain");
    http_json_send_string_field (FS("app_bundle_upload_accept"), "");
    http_json_send_string_field (FS("local_esp_update_accept"), ".bin,application/octet-stream");
    http_json_send_string_field (FS("local_stm32_upload_accept"), ".hex");
    http_json_send_string_field (FS("fs_show_base_url"), "/api/fs_show?filename=");
    http_json_send_string_field (FS("fs_remove_base_url"), "/api/fs_remove?filename=");
    http_json_send_string_field (FS("local_stm32_upload_url"), "/api/local_stm32_upload");
    http_json_send_string_field (FS("local_stm32_flash_url"), "/api/local_stm32_flash");
    http_json_send_string_field (FS("local_esp_update_url"), "/api/local_esp_update");
    http_json_send_string_field (FS("local_esp_restart_url"), "/api/local_esp_restart");
    http_json_send_string_field (FS("stm32_log_url"), "/api/stm32_log");
    http_json_send_string_field (FS("stm32_log_clear_url"), "/api/stm32_log_clear");
    http_json_send_string_field (FS("update_progress_url"), "/api/update_progress");
    http_json_send_string_field (FS("network_scan_url"), "/api/network_scan");
    http_json_send_string_field (FS("overlay_icons_url"), "/api/overlay_icons");
    http_json_send_string_field (FS("live_display_color_url"), "/api/live_display_color");
    http_json_send_string_field (FS("eeprom_settings_set_url"), "/api/eeprom_settings_set");
    http_json_send_string_field (FS("display_dim_level_set_url"), "/api/display_dim_level_set");
    http_json_send_string_field (FS("ambilight_dim_level_set_url"), "/api/ambilight_dim_level_set");
    http_json_send_string_field (FS("animation_profile_set_url"), "/api/animation_profile_set");
    http_json_send_string_field (FS("animation_profile_default_url"), "/api/animation_profile_default");
    http_json_send_string_field (FS("color_animation_profile_set_url"), "/api/color_animation_profile_set");
    http_json_send_string_field (FS("color_animation_profile_default_url"), "/api/color_animation_profile_default");
    http_json_send_string_field (FS("ambilight_mode_profile_set_url"), "/api/ambilight_mode_profile_set");
    http_json_send_string_field (FS("ambilight_mode_profile_default_url"), "/api/ambilight_mode_profile_default");
    http_json_send_string_field (FS("tft_flags_set_url"), "/api/tft_flags_set");
    http_json_send_string_field (FS("fs_upload_icon_target"), http_get_fs_upload_filename (POST_ICON_FILE));
    http_json_send_string_field (FS("fs_upload_weather_target"), http_get_fs_upload_filename (POST_ICON_WEATHER_FILE));
    http_json_send_string_field (FS("fs_upload_tables_target"), http_get_fs_upload_filename (POST_TABLES_FILE));
    http_json_send_string_field (FS("fs_upload_display_target"), http_get_fs_upload_filename (POST_DISPLAY_FILE));
    http_json_send_string_field (FS("fs_upload_tables_prefix"), http_get_tables_family_prefix ());
    http_json_send_string_field (FS("asset_prefix"), http_get_asset_prefix ());
    http_json_send_bool_field (FS("layout_is_12h"), (hardware_configuration & HW_WC_MASK) == HW_WC_12H);
    http_json_send_string_field (FS("hardware_label"), http_get_hardware_label ());
    http_json_send_string_field (FS("processor_label"), http_get_processor_label ());
    http_json_send_string_field (FS("board_label"), http_get_board_label ());
    http_json_send_string_field (FS("frequency_label"), http_get_frequency_label ());
    http_json_send_string_field (FS("oscillator_label"), http_get_oscillator_label ());
    http_json_send_string_field (FS("display_label"), http_get_display_label ());
    http_json_send_string_field (FS("display_led_mode"), http_get_display_led_mode ());
    http_json_send_bool_field (FS("display_has_tft"), (hardware_configuration & HW_LED_MASK) == HW_LED_TFTLED_RGB_LED);
    http_json_send_bool_field (FS("display_has_white_channel"), (hardware_configuration & HW_LED_MASK) == HW_LED_SK6812_RGBW_LED);
    http_json_send_string_field (FS("default_layout_preview_file"), http_get_default_layout_preview_filename ());
    http_json_send_uint_field (FS("default_layout_columns"), (unsigned long) http_get_default_layout_columns ());
    http_json_send_string_field (FS("local_stm32_expected_filename"), stm32_default_filename);
    http_json_send_string_field (FS("esp_version"), ESP_VERSION);
    http_json_send_string_field (FS("esp_available"), new_esp_version);
    http_json_send_string_field (FS("app_available"), new_app_version);
    http_json_send_string_field (FS("wc_version"), get_strvar (VERSION_STR_VAR)->str);
    http_json_send_string_field (FS("wc_available"), new_wc_version);
    /* Die Release Notes werden erst HIER geholt und dabei unmittelbar in die Antwort
     * geschrieben. Frueher standen sie als 3072-Byte-String schon am Kopf der Funktion
     * im Heap und blieben dort, bis diese Zeile erreicht war. Der ausgehende Abruf
     * mitten in der eigenen Antwort ist in dieser Funktion nichts Neues: dreissig
     * Zeilen weiter unten holt WC_LIST_TXT seine Daten genauso, und httpclient.cpp
     * benutzt dafuer eine eigene WiFiClient-Instanz (httpclient.cpp:16), nicht
     * http_client.
     */
    if (update_server_blocked ())
    {
        /* Sperrfenster offen: Feld leer ausgeben statt die laufende Antwort um bis zu
         * drei Zeitgrenzen anzuhalten. Die Antwortform bleibt unveraendert.
         */
        http_json_send_string_field (FS("release_notes"), "");
    }
    else
    {
        unsigned long   notes_millis = millis ();
        int             notes_len;

        notes_len = http_json_send_remote_text_field (FS("release_notes"), update_host, update_path, RELEASENOTE_HTML, 3072);
        update_server_note (notes_len, millis () - notes_millis);
    }

    http_json_send_string_field (FS("stm32_default"), stm32_default_filename);
    http_send (FS(",\"stm32_files\":["));

    /* Ent-5 (F.3): Ohne Filter (65535 oder unbekannter Typ) bleibt die Liste ohnehin leer -
     * dann den Abruf ganz sparen statt bis zu drei Zeitgrenzen fuer ein feststehendes Ergebnis.
     */
    len = filter ? update_server_open (update_host, update_path, WC_LIST_TXT) : 0;

    if (len > 0)
    {
        char fname[MAX_UPDATE_FILENAME_LEN];
        int ch;
        int l = 0;

        while (len > 0)
        {
            ch = httpclient_read (&len);

            if (ch < 0)
            {
                break;                                  // Lesefehler oder Zeitgrenze: *lenp bleibt stehen, sonst dreht die Schleife endlos (L152)
            }

            if (ch != '\r' && ch != '\n' && ch != ' ' && ch != '\t' && l < MAX_UPDATE_FILENAME_LEN - 1)
            {
                fname[l++] = ch;
            }
            else if (ch == '\n')
            {
                int show_option;

                fname[l] = '\0';
                l = 0;

                show_option = http_remote_stm32_filename_matches (fname);  // Ent-5: dieselbe Pruefung wie Flash und Legacy-Liste; bei 65535 leer (AKF.8)

                if (show_option)
                {
                    if (! first)
                    {
                        http_send (FS(","));
                    }

                    http_send (FS("\""));
                    http_send_json_escaped (fname);
                    http_send (FS("\""));
                    first = 0;
                }
            }
        }

        httpclient_stop ();
    }

    http_send (FS("]}"));
    http_flush ();

    return 0;
}

static int
http_api_update_progress ()
{
    char buf[16];

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"active\":"));
    http_send (update_progress.active ? "true" : "false");
    http_send (FS(",\"type\":\""));
    http_send_json_escaped (update_progress.type);
    http_send (FS("\",\"state\":\""));
    http_send_json_escaped (update_progress.state);
    http_send (FS("\",\"message\":\""));
    http_send_json_escaped (update_progress.message);
    http_send (FS("\",\"progress_current\":"));
    sprintf (buf, "%u", (unsigned) update_progress.progress_current);
    http_send (buf);
    http_send (FS(",\"progress_total\":"));
    sprintf (buf, "%u", (unsigned) update_progress.progress_total);
    http_send (buf);
    http_send (FS(",\"error_code\":"));
    sprintf (buf, "%u", (unsigned) update_progress.error_code);
    http_send (buf);
    http_send (FS(",\"started_at\":"));
    sprintf (buf, "%u", (unsigned) update_progress.started_at);
    http_send (buf);
    http_send (FS(",\"updated_at\":"));
    sprintf (buf, "%u", (unsigned) update_progress.updated_at);
    http_send (buf);
    http_send (FS(",\"finished_at\":"));
    sprintf (buf, "%u", (unsigned) update_progress.finished_at);
    http_send (buf);
    http_send (FS("}"));
    http_flush ();
    return 0;
}

static int
http_api_device_ready ()
{
    /* Beide Werte zuerst einlesen, dann ausgeben: http_json_send_uint_field () baut
     * intern ein String-Objekt. Wuerde erst gesendet und dann gemessen, laege die
     * Messung hinter einer eigenen Heap-Anforderung.
     * device_ready ist bewusst der Ort: die billigste lesende Antwort im Geraet,
     * ohne LittleFS-Mount, ohne STM-Kommando, ohne Abfrage beim Update-Server.
     * max_free_block neben free_heap, weil eine gescheiterte Pufferanforderung auch
     * bei reichlich freiem, aber zerstueckeltem Speicher auftritt (L125).
     */
    unsigned long free_heap      = ESP.getFreeHeap ();
    unsigned long max_free_block = ESP.getMaxFreeBlockSize ();

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"ready\":true"));
    http_json_send_uint_field (FS("free_heap"), free_heap);
    http_json_send_uint_field (FS("max_free_block"), max_free_block);
    /* Verlorene Antwortbyte seit dem Start. Solange beide 0 sind, hat jede Antwort
     * dieses Geraets die Verbindung vollstaendig erreicht - vorher war das gar nicht
     * feststellbar, weil ein Verlust syntaktisch gueltiges JSON hinterlaesst.
     */
    http_json_send_uint_field (FS("write_lost_bytes"), (unsigned long) http_write_lost_bytes);
    http_json_send_uint_field (FS("write_lost_blocks"), (unsigned long) http_write_lost_blocks);
    http_json_send_uint_field (FS("write_lost_gone"), (unsigned long) http_write_lost_gone);        // C25: Teil von write_lost_blocks, Client weg
    http_json_send_uint_field (FS("write_lost_failed"), (unsigned long) http_write_lost_failed);    // C25: Teil von write_lost_blocks, Schreibversagen
    /* Angenommene Verbindungen ohne Anfrage (L160). Steigt no_request_aborts im
     * Takt der PWA-Zyklen, bricht die Gegenstelle ab; steigt no_request_timeouts,
     * haelt jemand Verbindungen auf Vorrat offen. Vorher war beides nur an einer
     * Serial-Zeile zu sehen, die den STM-Empfangsring belastete.
     */
    http_json_send_uint_field (FS("no_request_timeouts"), (unsigned long) http_no_request_timeouts);
    http_json_send_uint_field (FS("no_request_aborts"), (unsigned long) http_no_request_aborts);
    /* Wirksamkeit der Massnahmen aus L161: update_cache_hits zaehlt die Aufrufe von
     * /api/update_status, die OHNE ausgehende Versionsabrufe auskamen,
     * update_server_down_count, wie oft das Sperrfenster scharf gestellt wurde.
     */
    http_json_send_uint_field (FS("update_cache_hits"), (unsigned long) update_cache_hits);
    http_json_send_uint_field (FS("update_server_down_count"), (unsigned long) update_server_down_count);
    http_send (FS("}"));
    http_flush ();
    return 0;
}

static void
http_json_send_remote_update_support_fields ()
{
    http_json_send_bool_field (FS("remote_esp_update_api_supported"), 1);
    http_json_send_bool_field (FS("remote_stm32_flash_api_supported"), 1);
}

static void
http_json_send_remote_update_url_fields ()
{
    http_json_send_string_field (FS("remote_esp_update_url"), "/api/remote_esp_update");
    http_json_send_string_field (FS("remote_stm32_flash_url"), "/api/remote_stm32_flash");
}

static int
http_api_reconnect_probe ()
{
    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"ready\":true,\"display_power\":\""));
    http_send(get_numvar (DISPLAY_POWER_NUM_VAR) ? "on" : "off");
    http_send (FS("\",\"ambilight_power\":\""));
    http_send(get_numvar (DISPLAY_AMBILIGHT_POWER_NUM_VAR) ? "on" : "off");
    http_send (FS("\"}"));
    http_flush ();
    return 0;
}

static int
http_api_remote_esp_update ()
{
    STR_VAR *           sv;
    char *              update_host;
    char *              update_path;
    char                path[MAX_UPDATE_HOST_LEN + MAX_UPDATE_PATH_LEN + MAX_UPDATE_FILENAME_LEN + 3];
    uint32_t            flashsize = ESP.getFlashChipRealSize ();

    if (flashsize < 4194304)
    {
        http_send (FS("HTTP/1.0 400 Bad Request\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
        http_send (FS("{\"ok\":false,\"error\":1,\"message\":\"ESP-Flashgröße für OTA zu klein\"}"));
        http_flush ();
        return 0;
    }

    sv = get_strvar (UPDATE_HOST_VAR);
    update_host = sv->str;

    if (! update_host[0])
    {
        update_host = (char *) DEFAULT_UPDATE_HOST;
    }

    sv = get_strvar (UPDATE_PATH_VAR);
    update_path = sv->str;

    if (! update_path[0])
    {
        update_path = (char *) DEFAULT_UPDATE_PATH;
    }

    sprintf (path, "/%s/%s", update_path, ESP_WORDCLOCK_BIN);
    update_progress_begin ("esp", "starting", "ESP-Firmware-Update wird gestartet.");

    http_header ("Update", "40", "/app/");
    begin_box ("Update");
    http_send_FS ("<P><B>Updating ESP firmware '");
    http_send (path);
    http_send_FS ("', reconnecting in 40 seconds ...</B></BR>\r\n");
    end_box ();
    http_trailer ();
    http_flush ();

    while (http_client.available())                               // firefox claims about connection reset, if we do not read all characters
    {
        http_client.read();
    }

    http_client.stop();
    delay (200);
    update_progress_state ("reconnect_wait", "ESP-Firmware wird geladen. Gerät startet danach neu.");

    t_httpUpdate_return ret = ESPhttpUpdate.update (http_client, update_host, 80, path);

    switch(ret)
    {
        case HTTP_UPDATE_FAILED:
            Serial.println("HTTP update: failed");
            update_progress_fail (ESPhttpUpdate.getLastError (), ESPhttpUpdate.getLastErrorString ().c_str ());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP update: no updates");
            update_progress_complete ("Keine neue ESP-Firmware verfügbar.");
            break;

        case HTTP_UPDATE_OK:
            Serial.println("HTTP update: ok");                    // will never be called, because we are rebooting
            break;
    }

    return 0;
}

static int
http_api_update_table_files ()
{
    STR_VAR * sv;
    char * update_host;
    char * update_path;
    const char * tables_filter = (const char *) NULL;
    char * current_tables = tables_fname ();
    int len;
    int first = 1;

    sv = get_strvar (UPDATE_HOST_VAR);
    update_host = sv->str;

    if (! update_host[0])
    {
        update_host = (char *) DEFAULT_UPDATE_HOST;
    }

    sv = get_strvar (UPDATE_PATH_VAR);
    update_path = sv->str;

    if (! update_path[0])
    {
        update_path = (char *) DEFAULT_UPDATE_PATH;
    }

    if (hardware_configuration != 0xFFFF)
    {
        switch (hardware_configuration & HW_WC_MASK)
        {
            case HW_WC_24H: tables_filter = "wc24h-tables-"; break;
            case HW_WC_12H: tables_filter = "wc12h-tables-"; break;
            default:        tables_filter = (const char *) NULL; break;
        }
    }

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"current_table\":\""));
    http_send_json_escaped (current_tables ? current_tables : "");
    http_send (FS("\",\"table_files\":["));

    if (tables_filter)
    {
        len = httpclient (update_host, update_path, WC_TABLES_LIST_TXT);

        if (len > 0)
        {
            char fname[MAX_UPDATE_FILENAME_LEN];
            int ch;
            int l = 0;

            while (len > 0)
            {
                ch = httpclient_read (&len);

                if (ch < 0)
                {
                    break;                                  // Lesefehler oder Zeitgrenze: *lenp bleibt stehen, sonst dreht die Schleife endlos (L152)
                }

                if (ch != '\r' && ch != '\n' && ch != ' ' && ch != '\t' && l < MAX_UPDATE_FILENAME_LEN - 1)
                {
                    fname[l++] = ch;
                }
                else if (ch == '\n')
                {
                    fname[l] = '\0';
                    l = 0;

                    if (fname[0] && ! strncmp (fname, tables_filter, strlen (tables_filter)))
                    {
                        if (! first)
                        {
                            http_send (FS(","));
                        }

                        http_send (FS("\""));
                        http_send_json_escaped (fname);
                        http_send (FS("\""));
                        first = 0;
                    }
                }
            }

            httpclient_stop ();
        }
    }

    http_send (FS("]}"));
    http_flush ();

    return 0;
}

static void
reset_button (void)
{
    http_send_FS ("<form method=\"GET\" action=\"/flash_stm32_local\" style=\"display:inline\">"
                  "<button type=\"submit\" name=\"action\" value=\"reset\">Reset STM32</button>"
                  "</form>\r\n");

}

static uint_fast8_t
run_local_stm32_flash_response ()
{
    bool ok;

    http_header("Local Update", NULL, NULL);
    begin_box ("Local Update");

    for (int i = 0; i < 100; i++)
    {
        http_send ("          ");
    }

    LittleFS.begin ();
    update_progress_begin ("stm32", "starting", "Lokales STM32-Update wird vorbereitet.");
    ok = stm32_flash_from_local();
    LittleFS.remove ("stm32.hex");
    LittleFS.end ();

    if (ok)
    {
        update_progress_complete ("STM32-Flash abgeschlossen. Warte auf Reset.");
        http_send_FS ("Done. <font color=red>Please Reset your STM32 now!</font></B><BR>\r\n");
        reset_button ();
    }
    else if (! update_progress.error_code)
    {
        update_progress_fail (1, "Lokaler STM32-Flash fehlgeschlagen.");
    }

    end_box ();
    http_trailer ();
    http_flush ();
    return 0;
}

static uint_fast8_t
http_api_local_stm32_upload ()
{
    size_t      content_length = 0;
    uint32_t    flashsize = ESP.getFlashChipRealSize ();
    uint_fast8_t ok = 0;
    uint32_t    error_code = 0;
    char *      uploaded_name = http_get_param ("filename");

    if (flashsize < 1048576)
    {
        error_code = 1;
    }
    else if (! http_local_stm32_filename_matches (uploaded_name))
    {
        error_code = 2;
    }
    else if (read_post_headers (&content_length) && content_length > 0)
    {
        LittleFS.begin ();

        File f = LittleFS.open ("stm32.hex", "w+");

        if (f)
        {
            ok = read_request_body_to_file (f, content_length) ? 1 : 0;
            f.close ();

            if (! ok)
            {
                LittleFS.remove ("stm32.hex");
                error_code = 3;
            }
        }
        else
        {
            error_code = 4;
        }

        LittleFS.end ();
    }
    else
    {
        error_code = 5;
    }

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":"));
    http_send (ok ? "true" : "false");

    if (! ok)
    {
        http_send (FS(",\"error\":"));
        http_send (String (error_code).c_str ());
    }

    http_send (FS("}"));
    http_flush ();
    return 0;
}

static uint_fast8_t
http_api_local_stm32_flash ()
{
    uint_fast8_t exists = 0;

    LittleFS.begin ();
    exists = LittleFS.exists ("stm32.hex") ? 1 : 0;
    LittleFS.end ();

    if (exists)
    {
        return run_local_stm32_flash_response ();
    }

    http_send (FS("HTTP/1.0 404 Not Found\r\nContent-Type: text/plain\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("Keine lokale STM32-Datei vorhanden.\r\n"));
    http_flush ();
    return 0;
}

static int
http_api_remote_stm32_flash_send_result (uint_fast8_t ok, uint32_t error_code, uint_fast8_t stream_mode)
{
    if (stream_mode)
    {
        http_send (FS("{\"ok\":true,\"event\":\"result\",\"result_ok\":"));
        http_send (ok ? "true" : "false");

        if (! ok)
        {
            http_send (FS(",\"error\":"));
            http_send (String (error_code ? error_code : update_progress.error_code).c_str ());
            http_send (FS(",\"message\":\""));
            http_send_json_escaped (update_progress.message);
            http_send (FS("\""));
        }

        http_send (FS("}\n"));
        http_flush ();
        update_progress_stream_finish ();
        return 0;
    }

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":"));
    http_send (ok ? "true" : "false");

    if (! ok)
    {
        http_send (FS(",\"error\":"));
        http_send (String (error_code ? error_code : update_progress.error_code).c_str ());
        http_send (FS(",\"message\":\""));
        http_send_json_escaped (update_progress.message);
        http_send (FS("\""));
    }

    http_send (FS("}"));
    http_flush ();
    return 0;
}

static int
http_api_remote_stm32_flash ()
{
    STR_VAR *       sv;
    char *          update_host;
    char *          update_path;
    char *          filename = http_get_param ("filename");
    char *          stream_param = http_get_param ("stream");
    uint32_t        flashsize = ESP.getFlashChipRealSize ();
    uint_fast8_t    ok = 0;
    uint32_t        error_code = 0;
    uint_fast8_t    stream_mode = 0;

    if (*stream_param == '1' ||
        *stream_param == 'y' ||
        *stream_param == 'Y' ||
        ! strcmp (stream_param, "true"))
    {
        stream_mode = 1;
    }

    if (stream_mode)
    {
        http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/x-ndjson\r\nCache-Control: no-cache\r\n\r\n"));
        http_flush ();
        update_progress_stream_start ();
    }

    if (flashsize < 4194304)
    {
        error_code = 1;
    }
    else if (! http_remote_stm32_filename_matches (filename))
    {
        error_code = 2;
    }
    else
    {
        sv = get_strvar (UPDATE_HOST_VAR);
        update_host = sv->str;

        if (! update_host[0])
        {
            update_host = (char *) DEFAULT_UPDATE_HOST;
        }

        sv = get_strvar (UPDATE_PATH_VAR);
        update_path = sv->str;

        if (! update_path[0])
        {
            update_path = (char *) DEFAULT_UPDATE_PATH;
        }

        Serial.print ("Flash STM32: http://");
        Serial.print (update_host);
        Serial.print ("/");
        Serial.print (update_path);
        Serial.print ("/");
        Serial.println (filename);

        delay (200);
        update_progress_begin ("stm32", "starting", "STM32-Update wird vorbereitet.");
        ok = stm32_flash_from_server (update_host, update_path, filename) ? 1 : 0;
        delay (200);
        Serial.println ("End of flashmode\r\n");
        Serial.flush ();

        if (ok)
        {
            update_progress_complete ("STM32-Flash abgeschlossen. Warte auf Reset.");
        }
        else if (! update_progress.error_code)
        {
            update_progress_fail (1, "STM32-Flash fehlgeschlagen.");
        }
    }

    return http_api_remote_stm32_flash_send_result (ok, error_code, stream_mode);
}

static uint_fast8_t
http_api_local_esp_update ()
{
    size_t      content_length = 0;
    uint32_t    max_sketch_space = (ESP.getFreeSketchSpace () - 0x1000) & 0xFFFFF000;
    uint_fast8_t ok = 0;
    uint16_t    error_code = 0;
    char *      uploaded_name = http_get_param ("filename");
    size_t      bytes_written = 0;
    size_t      skipped = 0;
    size_t      first_chunk_len = 0;
    uint_fast8_t begin_ok = 0;
    uint_fast8_t stream_ok = 0;
    uint_fast8_t end_ok = 0;
    size_t      progress_before_end = 0;
    size_t      remaining_before_end = 0;
    uint8_t     first_chunk[8];

    if (! http_local_esp_filename_matches (uploaded_name))
    {
        error_code = 1002;
    }
    else if (read_post_headers (&content_length) && content_length > 0 && content_length < max_sketch_space)
    {
        update_progress_begin ("esp", "starting", "Lokales ESP-Update wird vorbereitet.");
        Update.runAsync (true);
        skipped = skip_leading_body_newlines ();

        if (Update.begin (max_sketch_space, U_FLASH))
        {
            begin_ok = 1;

            while (first_chunk_len < sizeof (first_chunk) && first_chunk_len < content_length)
            {
                unsigned long start_millis = millis ();

                while (! http_client.available () && (millis () - start_millis) < READ_BODY_TIMEOUT)
                {
                    yield ();
                }

                if (! http_client.available ())
                {
                    break;
                }

                int ch = http_client.read ();

                if (ch < 0)
                {
                    break;
                }

                first_chunk[first_chunk_len++] = (uint8_t) ch;
            }

            if (first_chunk_len > 0)
            {
                bytes_written = Update.write (first_chunk, first_chunk_len);
            }

            if (bytes_written == first_chunk_len)
            {
                bytes_written += read_request_body_to_update (content_length - first_chunk_len);
            }

            stream_ok = bytes_written == content_length ? 1 : 0;
            progress_before_end = Update.progress ();
            remaining_before_end = Update.remaining ();

            if (stream_ok && Update.end (true))
            {
                end_ok = 1;
                ok = 1;
                update_progress_state ("reconnect_wait", "ESP-Firmware wurde geschrieben. Gerät startet neu.");
            }
            else
            {
                error_code = Update.getError ();
                Update.end ();
            }
        }
        else
        {
            error_code = Update.getError ();
        }
    }
    else if (content_length >= max_sketch_space)
    {
        error_code = 1001;
    }
    else
    {
        error_code = 1000;
    }

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":"));
    http_send (ok ? "true" : "false");
    http_send (FS(",\"error\":"));
    char errbuf[16];
    sprintf (errbuf, "%u", error_code);
    http_send (errbuf);
    http_send (FS(",\"detail\":\""));
    char chunkbuf[32];
    int chunkbuf_len = 0;
    for (size_t i = 0; i < first_chunk_len && chunkbuf_len < (int) sizeof (chunkbuf) - 3; i++)
    {
        chunkbuf_len += sprintf (chunkbuf + chunkbuf_len, "%02X", first_chunk[i]);
    }
    char detailbuf[192];
    sprintf (detailbuf, "len=%u max=%u skip=%u first=%s begin=%u written=%u progress=%u remaining=%u stream=%u end=%u", (unsigned) content_length, (unsigned) max_sketch_space, (unsigned) skipped, chunkbuf_len ? chunkbuf : "--", begin_ok, (unsigned) bytes_written, (unsigned) progress_before_end, (unsigned) remaining_before_end, stream_ok, end_ok);
    http_send_json_escaped (detailbuf);
    http_send (FS("\""));
    http_send (FS("}"));
    http_flush ();

    if (! ok)
    {
        update_progress_fail (error_code ? error_code : 1, "Lokales ESP-Update fehlgeschlagen.");
    }

    return 0;
}

static uint_fast8_t
http_api_local_esp_restart ()
{
    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true}"));
    http_flush ();
    delay (500);
    ESP.restart ();
    return 0;
}

static uint_fast8_t
flash_stm32_local (bool post = false)
{
    char *              action;
    int                 do_reset = 0;
    uint32_t            flashsize;

    flashsize = ESP.getFlashChipRealSize ();

    action = http_get_param ("action");

    if (*action)
    {
        if (! strcmp (action, "reset"))
        {
            do_reset = 1;
        }
    }

    if (flashsize >= 1048576)
    {
        if (post)
        {
            String line;

            // find the line with content type and boundary string
            String boundary;

            while (read_line(line))
            {
                if (line.startsWith("Content-Type:"))
                {
                    int n = line.indexOf("boundary=");
                    if (n > 0)
                    {
                        boundary = "--" + line.substring(n + strlen("boundary="));
                    }
                    break;
                }
            }

            // no boundary? something is wrong
            if (boundary == "")
            {
                http_header("Local Update", NULL, NULL);
                begin_box ("Local Update");
                http_send_FS ("<font color='red'>Wrong http header</font><br/>");
            }
            else
            {
                // read until a boundary has been found
                while(read_line(line) && line != boundary)
                {
                    ;
                }

                // read the empty line
                while (read_line(line) && line != "")
                {
                    ;
                }

                LittleFS.begin();
                File f = LittleFS.open("stm32.hex", "w+");
                bool write_ok = true;

                if (f)
                {
                    // read and save file content until the boundary has been found
                    while (read_line(line))
                    {
                        // boundary found, end of file
                        if (line == boundary)
                        {
                            break;
                        }
                        /* Ungeprueft wanderte eine bei vollem LittleFS abgeschnittene
                         * stm32.hex anschliessend in den STM. Eine halbe Firmware ist
                         * schlimmer als gar keine - der Flashvorgang unterbleibt.
                         */
                        if (f.write((const unsigned char*)line.c_str(), line.length()) != line.length() ||
                            f.write('\n') != 1)
                        {
                            write_ok = false;
                            break;
                        }
                    }
    
                    f.close();

                    if (write_ok)
                    {
                        return run_local_stm32_flash_response ();
                    }

                    LittleFS.remove ("stm32.hex");
                    Serial.println ("- fs write short on stm32.hex upload");
                    Serial.flush ();
                    http_header("Local Update", NULL, NULL);
                    begin_box ("Local Update");
                    http_send_FS ("<font color='red'>Dateisystem voll &mdash; stm32.hex unvollstaendig, es wurde nicht geflasht.</font><br/>");
                }
                else
                {
                    http_header("Local Update", NULL, NULL);
                    begin_box ("Local Update");
                    http_send_FS ("<font color='red'>cannot open stm32.hex on filesystem for writing.</font><br/>");
                }
    
                LittleFS.end();
            }
            end_box ();
            http_trailer ();
            http_flush ();
            return 0;
        }
        else if (do_reset)
        {
            http_header ("Local Update", "20", "/");
            http_send ("<P><B>Resetting STM32, reconnecting in 20 seconds ...</B><BR>\r\n");
            end_box ();
            http_trailer ();
            http_flush ();
            delay (200);
            stm32_reset ();
            return 0;
        }

        http_header("Local Update", NULL, NULL);
        begin_box ("Local Update");

        http_send_FS (
                "<br/>Please select the STM32 firmware file<br/>"
                "<form method='post' action='flash_stm32_local' name='submit' enctype='multipart/form-data' style=\"display:inline\">"
                "<label class='custom-file-upload'><input type='file' name='fileField'>File...</label><br /><br />"
                "<input type='submit' class='button' name='submit' value='Update STM32'>"
                "</form>"
                );
        reset_button ();
    }
    else
    {
        http_header("Local Update", NULL, NULL);
        begin_box ("Local Update");
        http_send_FS ("<font color=red><B>Flash size of ESP8266 is too small for Local Update over OTA</B></font><BR>\r\n");
    }

    end_box ();
    http_trailer();
    http_flush ();
    return 0;
}

static int
http_api_stm32_log ()
{
    uint16_t idx;
    uint16_t count = stm32_log_get_count ();

    http_send (FS("HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n"));
    http_send (FS("{\"ok\":true,\"count\":"));
    http_send (String(count));
    http_send (FS(",\"lines\":["));

    for (idx = 0; idx < count; idx++)
    {
        if (idx > 0)
        {
            http_send (FS(","));
        }

        http_send (FS("\""));
        http_send_escaped (stm32_log_get_line (idx), HTTP_ESCAPE_JSON_LOG);       // C23: UTF-8 und vollstaendig maskiert
        http_send (FS("\""));
    }

    http_send (FS("]}"));
    http_flush ();
    return 0;
}

static int
http_api_stm32_log_clear ()
{
    stm32_log_clear ();
    http_json_ok ();
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * http server
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
http (const char * path, const char * const_param)
{
    char param[256];

    strncpy (param, const_param, 255);
    param[255] = '\0';

    char *          p;
    int             rtc = 0;

    // log_printf ("http path: '%s'\r\n", path);

    for (p = param; *p; p++)
    {
        if (*p == '+')                                      // plus must be mapped to space if GET method
        {
            *p = ' ';
        }
    }

    // log_printf ("http parameters: '%s'\r\n", param);

    if (param[0])
    {
        http_set_params (param);
    }
    else
    {
        http_set_params ((char *) NULL);
    }

    if (hardware_configuration == 0xFFFF)
    {
        hardware_configuration = get_numvar (HARDWARE_CONFIGURATION_NUM_VAR);

        switch (hardware_configuration & HW_WC_MASK)
        {
            case HW_WC_24H:
                pgm_name        = "WordClock";
                hardware        = "WC24h";
                break;
            case HW_WC_12H:
                pgm_name        = "WordClock";
                hardware        = "WC12h";
                break;
            case HW_UCLOCK:
                pgm_name        = "uClock";
                hardware        = "uClock";
                break;
        }
    }

    if (! strcmp (path, "/") || ! strcmp (path, "/legacy") || ! strcmp (path, "/legacy/"))
    {
        rtc = http_main ();
    }
    else if (! strcmp (path, "/network"))
    {
        rtc = http_network ();
    }
    else if (! strcmp (path, "/temperature"))
    {
        rtc = http_temperature ();
    }
    else if (! strcmp (path, "/weather"))
    {
        rtc = http_weather ();
    }
    else if (! strcmp (path, "/ldr"))
    {
        rtc = http_ldr ();
    }
    else if (! strcmp (path, "/dispbright"))
    {
        rtc = http_display_brightness ();
    }
    else if (! strcmp (path, "/ambibright"))
    {
        rtc = http_ambilight_brightness ();
    }
    else if (! strcmp (path, "/display"))
    {
        rtc = http_display ();
    }
    else if (! strcmp (path, "/animations"))
    {
        rtc = http_animations ();
    }
    else if (! strcmp (path, "/overlays"))
    {
        rtc = http_overlays ();
    }
    else if (! strcmp (path, "/ambilight"))
    {
        rtc = http_ambilight ();
    }
    else if (! strcmp (path, "/timers"))
    {
        rtc = http_timers (0);
    }
    else if (! strcmp (path, "/atimers"))
    {
        rtc = http_timers (1);
    }
    else if (! strcmp (path, "/dfplayer"))
    {
        rtc = http_dfplayer ();
    }
    else if (! strcmp (path, "/tft"))
    {
        rtc = http_tft ();
    }
    else if (! strcmp (path, "/fs"))
    {
        rtc = http_fs ();
    }
    else if (! strcmp (path, "/update"))
    {
        rtc = http_update ();
    }
    else if (! strcmp (path, "/flash_stm32_local"))
    {
        rtc = flash_stm32_local ();
    }
    else if (! strcmp (path, PWA_PREFIX) || ! strcmp (path, PWA_PREFIX "/") ||
             ! strncmp (path, PWA_PREFIX "/", strlen (PWA_PREFIX "/")))
    {
        rtc = http_app (path);
    }
    else if (! strcmp (path, "/get_settings"))
    {
        rtc = http_get_settings ();
    }
    else if (! strcmp (path, "/display_power"))
    {
        rtc = http_get_display_power ();
    }
    else if (! strcmp (path, "/ambilight_power"))
    {
        rtc = http_get_ambilight_power ();
    }
    else if (! strcmp (path, "/api/display_power_set"))
    {
        rtc = http_api_display_power_set ();
    }
    else if (! strcmp (path, "/api/ambilight_power_set"))
    {
        rtc = http_api_ambilight_power_set ();
    }
    else if (! strcmp (path, "/api/display_brightness_set"))
    {
        rtc = http_api_display_brightness_set ();
    }
    else if (! strcmp (path, "/api/auto_brightness_set"))
    {
        rtc = http_api_auto_brightness_set ();
    }
    else if (! strcmp (path, "/api/display_mode_set"))
    {
        rtc = http_api_display_mode_set ();
    }
    else if (! strcmp (path, "/api/display_it_is_set"))
    {
        rtc = http_api_display_it_is_set ();
    }
    else if (! strcmp (path, "/api/ticker_set"))
    {
        rtc = http_api_ticker_set ();
    }
    else if (! strcmp (path, "/api/date_ticker_format_set"))
    {
        rtc = http_api_date_ticker_format_set ();
    }
    else if (! strcmp (path, "/api/ticker_deceleration_set"))
    {
        rtc = http_api_ticker_deceleration_set ();
    }
    else if (! strcmp (path, "/api/test_display"))
    {
        rtc = http_api_test_display ();
    }
    else if (! strcmp (path, "/api/weather_appid_set"))
    {
        rtc = http_api_weather_appid_set ();
    }
    else if (! strcmp (path, "/api/weather_city_set"))
    {
        rtc = http_api_weather_city_set ();
    }
    else if (! strcmp (path, "/api/weather_coordinates_set"))
    {
        rtc = http_api_weather_coordinates_set ();
    }
    else if (! strcmp (path, "/api/weather_get_now"))
    {
        rtc = http_api_weather_get_now ();
    }
    else if (! strcmp (path, "/api/weather_get_forecast"))
    {
        rtc = http_api_weather_get_forecast ();
    }
    else if (! strcmp (path, "/api/network_scan"))
    {
        rtc = http_api_network_scan ();
    }
    else if (! strcmp (path, "/api/eeprom_settings"))
    {
        rtc = http_api_eeprom_settings ();
    }
    else if (! strcmp (path, "/api/eeprom_settings_set"))
    {
        rtc = http_api_eeprom_settings_set ();
    }
    else if (! strcmp (path, "/api/stm32_log"))
    {
        rtc = http_api_stm32_log ();
    }
    else if (! strcmp (path, "/api/stm32_log_clear"))
    {
        rtc = http_api_stm32_log_clear ();
    }
    else if (! strcmp (path, "/api/live_display_color"))
    {
        rtc = http_api_live_display_color ();
    }
    else if (! strcmp (path, "/api/network_client_set"))
    {
        rtc = http_api_network_client_set ();
    }
    else if (! strcmp (path, "/api/network_ap_set"))
    {
        rtc = http_api_network_ap_set ();
    }
    else if (! strcmp (path, "/api/network_timeserver_set"))
    {
        rtc = http_api_network_timeserver_set ();
    }
    else if (! strcmp (path, "/api/network_timezone_set"))
    {
        rtc = http_api_network_timezone_set ();
    }
    else if (! strcmp (path, "/api/network_summertime_set"))
    {
        rtc = http_api_network_summertime_set ();
    }
    else if (! strcmp (path, "/api/network_get_time"))
    {
        rtc = http_api_network_get_time ();
    }
    else if (! strcmp (path, "/api/network_wps"))
    {
        rtc = http_api_network_wps ();
    }
    else if (! strcmp (path, "/api/update_host_set"))
    {
        rtc = http_api_update_host_set ();
    }
    else if (! strcmp (path, "/api/update_path_set"))
    {
        rtc = http_api_update_path_set ();
    }
    else if (! strcmp (path, "/api/update_status"))
    {
        rtc = http_api_update_status ();
    }
    else if (! strcmp (path, "/api/device_ready"))
    {
        rtc = http_api_device_ready ();
    }
    else if (! strcmp (path, "/api/reconnect_probe"))
    {
        rtc = http_api_reconnect_probe ();
    }
    else if (! strcmp (path, "/api/settings_xml"))
    {
        rtc = http_api_settings_xml ();
    }
    else if (! strcmp (path, "/api/display_power"))
    {
        rtc = http_api_display_power ();
    }
    else if (! strcmp (path, "/api/ambilight_power"))
    {
        rtc = http_api_ambilight_power ();
    }
    else if (! strcmp (path, "/api/power_status"))
    {
        rtc = http_api_power_status ();
    }
    else if (! strcmp (path, "/api/remote_esp_update"))
    {
        rtc = http_api_remote_esp_update ();
    }
    else if (! strcmp (path, "/api/update_progress"))
    {
        rtc = http_api_update_progress ();
    }
    else if (! strcmp (path, "/api/update_table_files"))
    {
        rtc = http_api_update_table_files ();
    }
    else if (! strcmp (path, "/api/update_download_assets"))
    {
        rtc = http_api_update_download_assets ();
    }
    else if (! strcmp (path, "/api/update_download_table"))
    {
        rtc = http_api_update_download_table ();
    }
    else if (! strcmp (path, "/api/remote_stm32_flash"))
    {
        rtc = http_api_remote_stm32_flash ();
    }
    else if (! strcmp (path, "/api/maintenance_format_fs"))
    {
        rtc = http_api_maintenance_format_fs ();
    }
    else if (! strcmp (path, "/api/maintenance_reset_stm32"))
    {
        rtc = http_api_maintenance_reset_stm32 ();
    }
    else if (! strcmp (path, "/api/maintenance_reset_eeprom"))
    {
        rtc = http_api_maintenance_reset_eeprom ();
    }
    else if (! strcmp (path, "/api/datetime_set"))
    {
        rtc = http_api_datetime_set ();
    }
    else if (! strcmp (path, "/api/learn_ir"))
    {
        rtc = http_api_learn_ir ();
    }
    else if (! strcmp (path, "/api/ir_codes_request"))
    {
        rtc = http_api_ir_codes_request ();
    }
    else if (! strcmp (path, "/api/ir_codes_get"))
    {
        rtc = http_api_ir_codes_get ();
    }
    else if (! strcmp (path, "/api/ir_code_set"))
    {
        rtc = http_api_ir_code_set ();
    }
    else if (! strcmp (path, "/api/temperature_display"))
    {
        rtc = http_api_temperature_display ();
    }
    else if (! strcmp (path, "/api/temperature_rtc_correction_set"))
    {
        rtc = http_api_temperature_rtc_correction_set ();
    }
    else if (! strcmp (path, "/api/temperature_ds18xx_correction_set"))
    {
        rtc = http_api_temperature_ds18xx_correction_set ();
    }
    else if (! strcmp (path, "/api/ldr_min_set"))
    {
        rtc = http_api_ldr_min_set ();
    }
    else if (! strcmp (path, "/api/ldr_min_value_set"))
    {
        rtc = http_api_ldr_min_value_set ();
    }
    else if (! strcmp (path, "/api/ldr_max_set"))
    {
        rtc = http_api_ldr_max_set ();
    }
    else if (! strcmp (path, "/api/ldr_max_value_set"))
    {
        rtc = http_api_ldr_max_value_set ();
    }
    else if (! strcmp (path, "/api/animation_mode_set"))
    {
        rtc = http_api_animation_mode_set ();
    }
    else if (! strcmp (path, "/api/color_animation_mode_set"))
    {
        rtc = http_api_color_animation_mode_set ();
    }
    else if (! strcmp (path, "/api/animation_profile_set"))
    {
        rtc = http_api_animation_profile_set ();
    }
    else if (! strcmp (path, "/api/animation_profile_default"))
    {
        rtc = http_api_animation_profile_default ();
    }
    else if (! strcmp (path, "/api/color_animation_profile_set"))
    {
        rtc = http_api_color_animation_profile_set ();
    }
    else if (! strcmp (path, "/api/color_animation_profile_default"))
    {
        rtc = http_api_color_animation_profile_default ();
    }
    else if (! strcmp (path, "/api/display_dim_level_set"))
    {
        rtc = http_api_display_dim_level_set ();
    }
    else if (! strcmp (path, "/api/ambilight_dim_level_set"))
    {
        rtc = http_api_ambilight_dim_level_set ();
    }
    else if (! strcmp (path, "/api/tft_flags_set"))
    {
        rtc = http_api_tft_flags_set ();
    }
    else if (! strcmp (path, "/api/display_use_rgbw_set"))
    {
        rtc = http_api_display_use_rgbw_set ();
    }
    else if (! strcmp (path, "/api/ambilight_brightness_set"))
    {
        rtc = http_api_ambilight_brightness_set ();
    }
    else if (! strcmp (path, "/api/ambilight_mode_set"))
    {
        rtc = http_api_ambilight_mode_set ();
    }
    else if (! strcmp (path, "/api/ambilight_leds_set"))
    {
        rtc = http_api_ambilight_leds_set ();
    }
    else if (! strcmp (path, "/api/ambilight_offset_set"))
    {
        rtc = http_api_ambilight_offset_set ();
    }
    else if (! strcmp (path, "/api/ambilight_mode_profile_set"))
    {
        rtc = http_api_ambilight_mode_profile_set ();
    }
    else if (! strcmp (path, "/api/ambilight_mode_profile_default"))
    {
        rtc = http_api_ambilight_mode_profile_default ();
    }
    else if (! strcmp (path, "/api/display_color_set"))
    {
        rtc = http_api_display_color_set ();
    }
    else if (! strcmp (path, "/api/ambilight_color_set"))
    {
        rtc = http_api_ambilight_color_set ();
    }
    else if (! strcmp (path, "/api/marker_color_set"))
    {
        rtc = http_api_marker_color_set ();
    }
    else if (! strcmp (path, "/api/sync_ambilight_set"))
    {
        rtc = http_api_sync_ambilight_set ();
    }
    else if (! strcmp (path, "/api/sync_markers_set"))
    {
        rtc = http_api_sync_markers_set ();
    }
    else if (! strcmp (path, "/api/fade_clock_seconds_set"))
    {
        rtc = http_api_fade_clock_seconds_set ();
    }
    else if (! strcmp (path, "/api/ambilight_markers_set"))
    {
        rtc = http_api_ambilight_markers_set ();
    }
    else if (! strcmp (path, "/api/ambilight_online_set"))
    {
        rtc = http_api_ambilight_online_set ();
    }
    else if (! strcmp (path, "/api/dfplayer_volume_set"))
    {
        rtc = http_api_dfplayer_volume_set ();
    }
    else if (! strcmp (path, "/api/dfplayer_mode_set"))
    {
        rtc = http_api_dfplayer_mode_set ();
    }
    else if (! strcmp (path, "/api/dfplayer_bell_flags_set"))
    {
        rtc = http_api_dfplayer_bell_flags_set ();
    }
    else if (! strcmp (path, "/api/dfplayer_speak_cycle_set"))
    {
        rtc = http_api_dfplayer_speak_cycle_set ();
    }
    else if (! strcmp (path, "/api/dfplayer_silence_start_set"))
    {
        rtc = http_api_dfplayer_silence_start_set ();
    }
    else if (! strcmp (path, "/api/dfplayer_silence_stop_set"))
    {
        rtc = http_api_dfplayer_silence_stop_set ();
    }
    else if (! strcmp (path, "/api/dfplayer_play"))
    {
        rtc = http_api_dfplayer_play ();
    }
    else if (! strcmp (path, "/api/dfplayer_alarm_set"))
    {
        rtc = http_api_dfplayer_alarm_set ();
    }
    else if (! strcmp (path, "/api/overlay_set"))
    {
        rtc = http_api_overlay_set ();
    }
    else if (! strcmp (path, "/api/overlay_display"))
    {
        rtc = http_api_overlay_display ();
    }
    else if (! strcmp (path, "/api/overlay_delete"))
    {
        rtc = http_api_overlay_delete ();
    }
    else if (! strcmp (path, "/api/timer_set"))
    {
        rtc = http_api_timer_set ();
    }
    else if (! strcmp (path, "/api/ambilight_timer_set"))
    {
        rtc = http_api_ambilight_timer_set ();
    }
    else if (! strcmp (path, "/api/overlay_icons"))
    {
        rtc = http_api_overlay_icons ();
    }
    else if (! strcmp (path, "/api/fs_info"))
    {
        rtc = http_api_fs_info ();
    }
    else if (! strcmp (path, "/api/fs_list"))
    {
        rtc = http_api_fs_list ();
    }
    else if (! strcmp (path, "/api/fs_show"))
    {
        rtc = http_api_fs_show ();
    }
    else if (! strcmp (path, "/api/fs_remove"))
    {
        rtc = http_api_fs_remove ();
    }
    else if (! strcmp (path, "/api/fs_upload_icon"))
    {
        rtc = http_api_fs_upload (POST_ICON_FILE);
    }
    else if (! strcmp (path, "/api/fs_upload_weather"))
    {
        rtc = http_api_fs_upload (POST_ICON_WEATHER_FILE);
    }
    else if (! strcmp (path, "/api/fs_upload_tables"))
    {
        rtc = http_api_fs_upload (POST_TABLES_FILE);
    }
    else if (! strcmp (path, "/api/fs_upload_display"))
    {
        rtc = http_api_fs_upload (POST_DISPLAY_FILE);
    }
    else if (! strcmp (path, "/api/app_file_upload"))
    {
        rtc = http_api_app_file_upload ();
    }
    else if (! strcmp (path, "/api/local_stm32_flash"))
    {
        rtc = http_api_local_stm32_flash ();
    }
    else if (! strcmp (path, "/api/local_esp_restart"))
    {
        rtc = http_api_local_esp_restart ();
    }
    else
    {
        const char * p = "HTTP/1.0 404 Not Found";
        http_send (p);
        http_send_FS ("\r\n\r\n404 Not Found\r\n");
        http_flush ();
    }

    return rtc;
}

void
http_post(const String& sPath)
{
    if (sPath == "/flash_stm32_local")
    {
        flash_stm32_local (true);
    }
    else if (sPath == "/fs-icon")
    {
        http_fs (POST_ICON_FILE);
    }
    else if (sPath == "/fs-icon-weather")
    {
        http_fs (POST_ICON_WEATHER_FILE);
    }
    else if (sPath == "/fs-tables")
    {
        http_fs (POST_TABLES_FILE);
    }
    else if (sPath == "/fs-display")
    {
        http_fs (POST_DISPLAY_FILE);
    }
    else if (sPath == "/api/fs_upload_icon")
    {
        http_api_fs_upload (POST_ICON_FILE);
    }
    else if (sPath == "/api/fs_upload_weather")
    {
        http_api_fs_upload (POST_ICON_WEATHER_FILE);
    }
    else if (sPath == "/api/fs_upload_tables")
    {
        http_api_fs_upload (POST_TABLES_FILE);
    }
    else if (sPath == "/api/fs_upload_display")
    {
        http_api_fs_upload (POST_DISPLAY_FILE);
    }
    else if (sPath == "/api/app_file_upload")
    {
        http_api_app_file_upload ();
    }
    else if (sPath == "/api/local_stm32_upload")
    {
        http_api_local_stm32_upload ();
    }
    else if (sPath == "/api/local_esp_update")
    {
        http_api_local_esp_update ();
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * http server
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
http_server_loop (void)
{
    String  sPath       = "";
    String  sParam      = "";
    String  sCmd        = "";
    String  sGetstart   = "GET ";
    String  sPoststart  = "POST ";
    String  sResponse   = "";
    int     start_position;
    int     end_position_space;
    int     end_position_question;
    String  sRemoteIp;
    String  sHeaderLine;

    http_client = http_server.accept();                                     // check if a client has connected

    if (!http_client)                                                       // wait until the client sends some data
    {
        return;
    }

    sRemoteIp = http_client.remoteIP().toString();
    http_clear_request_user_agent ();
    http_clear_request_fetch_dest ();                       // sonst wirkt der vorige Request nach
    http_write_broken = 0;                                  // gilt je Verbindung, nicht ueber den Lauf

#if HTTP_DEBUG_CLIENT_LOG                               // rund 35 Byte je Request auf die STM-UART, siehe oben
    Serial.print ("- new client");
    if (sRemoteIp.length ())
    {
        Serial.print (" from ");
        Serial.print (sRemoteIp);
    }
    Serial.println ();
    Serial.flush ();
#endif

    /* Warten auf die erste Anfragezeile. Hier steckten drei Defekte (BEFUNDE.md L160):
     *
     * (a) Der Zweig kehrte OHNE http_client.stop () zurueck. Die Verbindung blieb
     *     offen, bis der naechste accept () das globale http_client ueberschrieb -
     *     der Client sah dann ein FIN ohne Antwort. Genau das Bild aus L149
     *     (BadStatusLine nach 31,5 s). Die beiden Nachbarzweige darunter rufen
     *     stop (), dieser nicht.
     * (b) Ohne connected ()-Test wurden die vollen 250 ms auch dann abgesessen, wenn
     *     die Gegenstelle laengst zurueckgesetzt hatte. Jeder AbortController-Abbruch
     *     der PWA tut das, und Chrome oeffnet Verbindungen auf Vorrat: vier
     *     abgebrochene Verbindungen waren eine Sekunde reiner Leerlauf im Hauptloop.
     * (c) "millis () < ultimeout" bricht beim Ueberlauf alle 49,7 Tage - dann wird
     *     250 ms lang JEDE Anfrage abgewiesen. Die Differenzbildung ist
     *     ueberlaufsicher und in dieser Datei schon zweimal so geschrieben.
     *
     * available () wird zuerst geprueft: connected () liefert zwar true, solange noch
     * Daten anstehen (WiFiClient.cpp:332), aber diese Reihenfolge ist auch dann
     * richtig, wenn sich diese Zusage im Core einmal aendert.
     */
    unsigned long start_millis    = millis ();
    uint_fast8_t  request_arrived = 0;

    while ((millis () - start_millis) < HTTP_FIRST_LINE_TIMEOUT)
    {
        if (http_client.available ())
        {
            request_arrived = 1;
            break;
        }

        if (! http_client.connected ())
        {
            break;                                      // (b) Gegenstelle weg, kein Request mehr zu erwarten
        }

        delay (1);
    }

    if (! request_arrived)
    {
        /* Hier stand eine UNBEDINGTE Serial-Zeile: rund 30 Byte je verworfener
         * Verbindung auf die STM-UART, deren Empfangsring 256 Byte gross ist und bei
         * Ueberlauf still verwirft (uart-driver.h:698). Getroffen hat sie genau die
         * Verbindungen, die Chrome auf Vorrat oeffnet und nie benutzt - mehrere je
         * Seitenaufruf. Statt der Zeile zwei Zaehler in /api/device_ready; fuer die
         * Fehlersuche bleibt die Ausgabe ueber HTTP_DEBUG_CLIENT_LOG erreichbar.
         */
        if (http_client.connected ())
        {
            if (http_no_request_timeouts < 0xFFFF)
            {
                http_no_request_timeouts++;
            }
        }
        else
        {
            if (http_no_request_aborts < 0xFFFF)
            {
                http_no_request_aborts++;
            }
        }

#if HTTP_DEBUG_CLIENT_LOG
        Serial.println ("- client connection time-out!");
        Serial.flush ();
#endif
        http_client.stop ();                            // (a) sonst sieht der Client ein FIN ohne Antwort
        return;
    }

    http_client.setNoDelay(1);

    String sRequest = "";

    if (! http_read_request_line (sRequest, 300))                          // read the first line of the request with a short timeout
    {
        Serial.println ("- empty http request");
        Serial.flush ();
        while (http_client.available())
        {
            http_client.read();
        }
        http_client.stop();
        return;
    }

    if (sRequest == "")                                                     // stop client, if request is empty
    {
        Serial.println ("- empty http request");
        Serial.flush ();
        while (http_client.available())                                     // firefox claims about connection reset, if we do not read all characters
        {
            http_client.read();
        }
        http_client.stop();
        return;
    }

    // POST
    start_position = sRequest.indexOf(sPoststart);

    if (start_position == 0)
    {
        Serial.print ("- request");
        if (sRemoteIp.length ())
        {
            Serial.print (" ");
            Serial.print (sRemoteIp);
        }
        Serial.print (": ");
        Serial.println (sRequest);
        Serial.flush ();

        start_position += sPoststart.length ();
        end_position_space = sRequest.indexOf (" ", start_position);
        end_position_question = sRequest.indexOf ("?", start_position);

        if (end_position_space > 0)
        {
            if (end_position_question > 0)
            {
                char param[256];

                sPath  = sRequest.substring(start_position, end_position_question);
                sParam = sRequest.substring(end_position_question + 1, end_position_space);

                strncpy (param, sParam.c_str (), 255);
                param[255] = '\0';
                http_set_params (param);
            }
            else
            {
                sPath = sRequest.substring(start_position, end_position_space);
                http_set_params ((char *) NULL);
            }
        }
        http_post (sPath);
    }
    else // GET
    {
        while (read_line (sHeaderLine))
        {
            if (sHeaderLine == "")
            {
                if (http_client.available () && http_client.peek () == '\n')
                {
                    http_client.read ();
                }
                break;
            }

            if (sHeaderLine.startsWith ("User-Agent:"))
            {
                http_capture_request_user_agent (sHeaderLine);
            }
            else if (sHeaderLine.startsWith ("Sec-Fetch-Dest:"))
            {
                http_capture_request_fetch_dest (sHeaderLine);
            }
        }

        Serial.print ("- request");
        if (sRemoteIp.length ())
        {
            Serial.print (" ");
            Serial.print (sRemoteIp);
        }
        if (http_get_request_browser_label ())
        {
            Serial.print (" [");
            Serial.print (http_get_request_browser_label ());
            Serial.print ("]");
        }
        Serial.print (": ");
        Serial.println (sRequest);
        Serial.flush ();

        start_position = sRequest.indexOf(sGetstart);

        if (start_position >= 0)
        {
            start_position += sGetstart.length ();
            end_position_space = sRequest.indexOf (" ", start_position);
            end_position_question = sRequest.indexOf ("?", start_position);

            if (end_position_space > 0)                                         // parameters?
            {
                if (end_position_question > 0)
                {                                                               // yes
                    sPath  = sRequest.substring(start_position, end_position_question);
                    sParam = sRequest.substring(end_position_question + 1, end_position_space);
                }
                else
                {                                                               // no
                    sPath  = sRequest.substring(start_position, end_position_space);
                }
            }
        }

        http (sPath.c_str(), sParam.c_str());
    }

    if (http_write_broken)
    {
        /* Mindestens ein Pufferinhalt ist nicht in die Verbindung gelangt. Ein
         * regulaeres stop () sendet FIN, und weil diese Antworten ohne Content-Length
         * laufen, liest der Client das als vollstaendige Antwort - genau so wurde aus
         * einer verstuemmelten /api/update_status-Antwort gueltiges JSON mit 26
         * fehlenden Feldern. abort () sendet stattdessen RST; fetch (), curl und die
         * PWA melden dann einen Verbindungsfehler statt stiller Datenverluste.
         */
        http_client.abort();
        return;
    }

    http_client.flush();

    while (http_client.available())                                         // firefox claims about connection reset, if we do not read all characters
    {
        http_client.read();
    }

    http_client.stop();                                                     // stop client
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * http server begin
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
http_server_begin (void)
{
    http_server.begin();
}
