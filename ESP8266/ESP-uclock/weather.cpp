/*----------------------------------------------------------------------------------------------------------------------------------------
 * weather.cpp - weather stuff
 *
 * Copyright (c) 2016-2025 Frank Meyer - frank(at)uclockfli4l.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#include <ESP8266WiFi.h>
#include "weather.h"
#include "base.h"

WiFiClient openweather_client;

static int
round_up (char * degree)
{
    int   deg;
    int   i;
    bool  round_up = false;

    for (i = 0; degree[i]; i++)
    {
        if (degree[i] == '.')
        {
            if (degree[i + 1] >= '5')
            {
                round_up = true;
            }

            break;
        }
    }

    deg = atoi (degree);

    if (round_up)
    {
        deg++;
    }

    return deg;
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * utf8_truncate_len - Laenge auf die naechste UTF-8-Zeichengrenze zurueckziehen (C24, L249)
 *
 * parse_json() schneidet die Wetterbeschreibung bei max_len - 1 Byte hart ab. Faellt der Schnitt in
 * ein Mehrbyte-Zeichen, endet die Zeichenkette auf einem halben Zeichen -- genau die Eingabe, an der
 * convert_utf8_to_iso8859() vor L236 ueber die Pufferkante gelesen hat. Die Luecke dort ist zu, die
 * Quelle erzeugt ohne diese Korrektur aber weiter kaputte Eingaben.
 *
 * Geprueft wird nur das letzte angefangene Zeichen: vom letzten kopierten Byte rueckwaerts bis zum
 * Fuehrungsbyte, dann dessen erwartete Laenge gegen die tatsaechlich vorhandenen Bytes. Fehlt etwas,
 * faellt das ganze Zeichen weg -- der Text wird ein bis drei Byte kuerzer statt halb.
 *
 * Die Funktion wird NICHT nur bei gekuerzten Werten aufgerufen: Endet schon die Antwort der fremden
 * Website auf einem halben Zeichen, greift sie ebenso. Fuer vollstaendige Zeichenketten und fuer
 * reinen ASCII-Text ist sie wirkungslos.
 *
 * ES GIBT DIESE REGEL EIN ZWEITES MAL -- utf8_truncated_len() in vars.cpp (aus L46). Sie wird hier
 * bewusst nicht benutzt, weil ihr Vertrag ein anderer ist: Sie erwartet eine NUL-terminierte
 * Zeichenkette und ruft strlen() darauf. Der Wert in parse_json() ist aber ein Ausschnitt MITTEN
 * in der JSON-Antwort; strlen() liefe dort ueber den ganzen Rest des Puffers, und der Fall
 * "Antwort endet selbst auf einem halben Zeichen" bliebe offen (len <= maxlen gibt dort
 * unveraendert zurueck). Damit aus dem Doppel kein zweiter Wahrheitsanspruch wird, sind beide
 * Fassungen gegeneinander gerechnet worden: 27 Schnittlagen, 0 Abweichungen im ueberlappenden
 * Vertrag. Wer hier etwas aendert, rechnet erneut gegen vars.cpp.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static int
utf8_truncate_len (const char * str, int l)
{
    int     i;
    int     need;

    for (i = l - 1; i >= 0 && ((unsigned char) str[i] & 0xC0) == 0x80; i--)                 // Folgebytes ueberspringen
    {
        ;
    }

    if (i >= 0 && ((unsigned char) str[i] & 0x80))                                          // letztes Zeichen ist mehrbytig
    {
        if (((unsigned char) str[i] & 0xE0) == 0xC0)
        {
            need = 2;
        }
        else if (((unsigned char) str[i] & 0xF0) == 0xE0)
        {
            need = 3;
        }
        else if (((unsigned char) str[i] & 0xF8) == 0xF0)
        {
            need = 4;
        }
        else
        {
            need = 1;                                                                       // ungueltiges Fuehrungsbyte: unveraendert lassen
        }

        if (l - i < need)                                                                   // angefangen, aber nicht vollstaendig
        {
            l = i;
        }
    }

    return l;
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * parse_json - simple json parser
 *
 * ArduinoJson parser is too fat and fails with 9 forecast data lines
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static int
parse_json (const char * str, const char * pattern, int cnt, char * result, int max_len)
{
    int     len    = strlen (pattern);
    int     found  = 0;

    while (*str)
    {
        if (*str == '"' && ! strncmp (str + 1, pattern, len) && *(str + len + 1) == '"' && *(str + len + 2) == ':')
        {
            if (found == cnt)
            {
                int l = 0;

                str += len + 3;

                if (*str == '"')
                {
                    str++;

                    while (*(str + l) && *(str + l) != '"')
                    {
                        l++;
                    }
                }               
                else
                {
                    while (*(str + l) && ((*(str + l) >= '0' && *(str + l) <= '9') || *(str + l) == '.'))
                    {
                        l++;
                    }
                }               

                if (l > max_len - 1)
                {
                    l = max_len - 1;
                }

                l = utf8_truncate_len (str, l);                 // C24/L249: nie mitten in einem Zeichen schneiden

                strncpy (result, str, l);
                *(result + l) = '\0';
                return 1;
            }

            found++;
        }
        str++;
    }

    return 0;
}

#define MAX_LEN_COD             8
#define MAX_LEN_TEMP            8
#define MAX_LEN_DESCRIPTION     32
#define MAX_LEN_ICON            8

/* Rueckgabe der Parser (L338, Nachtrag M1 aus Review E.5): Genau eine Endzeile oder keine, und WELCHE.
 * Die Messzeile leitet ihr Ergebnis hieraus ab, NICHT aus dem Text der Endzeile.
 *   WEATHER_END_NONE   keine Endzeile gesendet (Icon verlangt, aber keines in der Antwort)
 *   WEATHER_END_OK     Endzeile in Erfolgsform: WEATHER/WEATHER_FC mit Wetter, WICON/WICON_FC mit Icon
 *   WEATHER_END_ERROR  Endzeile meldet einen Fehler: "... Error <cod>" (cod != 200) oder "... Parse Error"
 *                      (kein "cod" in der Antwort) -- in beiden Parsern, auch wenn ein Icon verlangt war
 */
#define WEATHER_END_NONE        0
#define WEATHER_END_OK          1
#define WEATHER_END_ERROR       2

/*----------------------------------------------------------------------------------------------------------------------------------------
 * parse_weather () - parse the answer for forecast and store the valuse in weather_fc struct
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static int
parse_weather (const char * answer, uint_fast8_t do_get_icon)
{
    char cod[MAX_LEN_COD];

    if (parse_json (answer, "cod", 0, cod, MAX_LEN_COD) == 1)
    {
        if (atoi (cod) == 200)
        {
            if (do_get_icon)
            {
                char icon[MAX_LEN_ICON];

                if (parse_json (answer, "icon", 0, icon, MAX_LEN_ICON) == 1)
                {
                    Serial.print("WICON ");
                    Serial.println (icon);
                }
                else
                {
                    return WEATHER_END_NONE;                                                            // keine Endzeile (L338)
                }
            }
            else
            {
                char  degree[MAX_LEN_TEMP];
                char  description[MAX_LEN_DESCRIPTION];
                int   deg;

                Serial.print ("WEATHER ");
                Serial.print ("Wetter heute: ");

                if (parse_json (answer, "temp", 0, degree, MAX_LEN_TEMP) == 1)
                {
                    deg = round_up (degree);
                    Serial.print (deg);
                    Serial.print (" Grad, ");
                }

                if (parse_json (answer, "description", 0, description, MAX_LEN_DESCRIPTION) == 1)
                {
                    char * description_iso8 = (char *) convert_utf8_to_iso8859 ((const unsigned char *) description);
                    Serial.print (description_iso8);
                }

                Serial.println ("");
            }
        }
        else
        {
            Serial.print ("WEATHER Wetter heute: Error ");
            Serial.println (cod);
            return WEATHER_END_ERROR;
        }
    }
    else
    {
        Serial.println ("WEATHER Wetter heute: Parse Error");
        return WEATHER_END_ERROR;
    }

    return WEATHER_END_OK;
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * parse_weather_fc () - parse the answer for forecast and store the valuse in weather_fc struct
 * Rueckgabe (beide Parser): WEATHER_END_NONE, _OK oder _ERROR, siehe dort (L338, M1).
 * we get 9 of max. 36 records:
 * 0 : current weather
 * 1 : current weather + 3h
 * 2 : current weather + 6h
 * ...
 * 8 : current weather + 24h
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static int
parse_weather_fc (const char * answer, uint_fast8_t do_get_icon)
{
    char cod[MAX_LEN_COD];

    if (parse_json (answer, "cod", 0, cod, MAX_LEN_COD) == 1)
    {
        if (atoi (cod) == 200)
        {
            if (do_get_icon)
            {
                char icon[MAX_LEN_ICON];

                if (parse_json (answer, "icon", 8, icon, MAX_LEN_ICON) == 1)                           // line index == 8
                {
                    Serial.print("WICON_FC ");
                    Serial.println (icon);
                }
                else
                {
                    return WEATHER_END_NONE;                                                            // keine Endzeile (L338)
                }
            }
            else
            {
                char  degree[MAX_LEN_TEMP];
                char  description[MAX_LEN_DESCRIPTION];
                int   deg;

                Serial.print ("WEATHER_FC ");
                Serial.print ("Wetter morgen: ");

                if (parse_json (answer, "temp", 8, degree, MAX_LEN_TEMP) == 1)                         // line index == 8
                {
                    deg = round_up (degree);
                    Serial.print (deg);
                    Serial.print (" Grad, ");
                }

                if (parse_json (answer, "description", 8, description, MAX_LEN_DESCRIPTION) == 1)      // line index == 8
                {
                    char * description_iso8 = (char *) convert_utf8_to_iso8859 ((const unsigned char *) description);
                    Serial.print (description_iso8);
                }

                Serial.println ("");
            }
        }
        else
        {
            Serial.print ("WEATHER_FC Wetter morgen: Error ");
            Serial.println (cod);
            return WEATHER_END_ERROR;
        }
    }
    else
    {
        Serial.println ("WEATHER_FC Wetter morgen: Parse Error");
        return WEATHER_END_ERROR;
    }

    return WEATHER_END_OK;
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * Fristen des Wetterabrufs (L338, Paket 2026-10-09 A1)
 *
 * WEATHER_TOTAL_TIMEOUT_MS ist NUR ein Sicherheitsnetz fuer einen Dienst, der nicht antwortet oder nicht
 * schliesst. Im Normalfall endet der Abruf am Verbindungsende (der ESP sendet "Connection: close"),
 * gemessen 0,13 bis 0,24 s. Gemessen ab dem Beginn von query_weather ().
 *
 * DIE FRIST HAENGT AN DER A2-FRIST IM STM (6 s ab dem Anstoss, var_send_buf () in src/vars/vars.c):
 * Wer sie hier anhebt, muss die STM-Seite mitziehen, sonst gibt der STM auf, waehrend der ESP noch liest.
 *
 * DNS, Verbindungsaufbau, Senden und Lesen teilen sich EINE Frist; jeder Schritt bekommt nur den Rest.
 * connect (hostname, ...) taugt dafuer nicht: Es begrenzt DNS und Verbindungsaufbau JE mit der vollen
 * Frist (Core 3.1.2, WiFiClient.cpp:130-137 und :145-163, ClientContext.h:129-159), die Frist wirkte
 * doppelt. Deshalb WiFi.hostByName (..., rest) und connect (ip, ...) getrennt.
 *
 * WEATHER_STOP_RESERVE_MS: stop () wartet bis WIFICLIENT_MAX_FLUSH_WAIT_MS (300 ms) auf die Quittung
 * des Gesendeten (WiFiClient.cpp:306-325). Die Arbeitsfrist endet um so viel frueher, damit der
 * Abruf in jedem Fall nach hoechstens WEATHER_TOTAL_TIMEOUT_MS zurueckkehrt.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#define WEATHER_TOTAL_TIMEOUT_MS        5000
#define WEATHER_STOP_RESERVE_MS         300
#define WEATHER_WORK_LIMIT_MS           (WEATHER_TOTAL_TIMEOUT_MS - WEATHER_STOP_RESERVE_MS)

#define WEATHER_LINE_NL                 0                                                           // Zeile mit '\n' beendet
#define WEATHER_LINE_CLOSED             1                                                           // Verbindungsende, Zeile ggf. ohne '\n'
#define WEATHER_LINE_TIMEOUT            2                                                           // Arbeitsfrist abgelaufen

/*----------------------------------------------------------------------------------------------------------------------------------------
 * weather_rest_ms () - verbleibende Arbeitsfrist, 0 = abgelaufen. Differenzbildung ist ueberlaufsicher.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static uint32_t
weather_rest_ms (uint32_t start_ms)
{
    uint32_t    elapsed = (uint32_t) millis () - start_ms;

    return (elapsed < WEATHER_WORK_LIMIT_MS) ? (WEATHER_WORK_LIMIT_MS - elapsed) : 0;
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * weather_read_line () - die EINE Lesehilfe des Wetterabrufs (L338)
 *
 * Liest Zeichen, solange available () || connected () und die Arbeitsfrist laeuft. Endet bei '\n',
 * beim Verbindungsende oder bei Fristablauf. Das Verbindungsende gilt erst, wenn NACH connected () == 0
 * auch available () 0 meldet (Rueckschritt ESP 3.2.27, G1 10.10.2026; Begruendung an der Stelle).
 * Die Zeile kommt wie bei readStringUntil ('\n') an: ohne
 * das '\n', ein '\r' davor bleibt stehen - der Parser bekommt dieselbe Zeichenkette wie bisher.
 *
 * Anders als readStringUntil () wartet sie nach dem Verbindungsende NICHT bis zur Frist
 * (Stream::timedRead prueft connected () nicht) - ein Koerper ohne '\n' ist damit sofort fertig.
 * In jedem Durchlauf ohne Zeichen gibt sie die Kontrolle ab (Soft-Watchdog).
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static int
weather_read_line (String & line, uint32_t start_ms)
{
    line = "";

    for (;;)
    {
        if (weather_rest_ms (start_ms) == 0)
        {
            return WEATHER_LINE_TIMEOUT;
        }

        if (openweather_client.available ())
        {
            int ch = openweather_client.read ();

            if (ch == '\n')
            {
                return WEATHER_LINE_NL;
            }

            if (ch >= 0)
            {
                line += (char) ch;
            }
        }
        else if (! openweather_client.connected ())
        {
            /* Rueckschritt ESP 3.2.27 (G1 10.10.2026): NOCHMAL nachsehen, bevor das Verbindungsende gilt.
             * available () erhebt den Puffer ZUERST und gibt erst DANACH per optimistic_yield (100) ab
             * (WiFiClient.cpp:246-257). Waehrend dieser Abgabe stellt der WLAN-Treiber die restlichen Segmente samt FIN zu -- die 0 von eben
             * ist dann veraltet. connected () meldet danach 0, obwohl Daten im Puffer liegen: state ()
             * zaehlt CLOSE_WAIT als CLOSED (ClientContext.h:363-371), und WiFiClient::connected () kehrt
             * in diesem Fall zurueck, ohne available () zu fragen (WiFiClient.cpp:327-333). Am Geraet
             * brach so jeder Koerper nach dem ersten 536-Byte-Segment ab (ESP 3.2.27).
             * Ist connected () einmal 0, kommt nichts mehr nach: Das FIN folgt den Daten in Reihenfolge,
             * und ClientContext::_recv () behaelt den Puffer beim FIN (ClientContext.h:600-615). Was
             * available () JETZT meldet, ist also endgueltig.
             */
            if (openweather_client.available ())
            {
                continue;
            }

            return WEATHER_LINE_CLOSED;
        }
        else
        {
            delay (1);                                                                              // Kontrolle abgeben
        }
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * weather_read_answer () - Kopf bis zur Leerzeile (hoechstens 20 Zeilen), dann eine Koerperzeile, unter
 * 10 Zeichen (Chunk-Laenge) die naechste. Parselogik wie bisher, nur ueber weather_read_line ().
 * Rueckgabe: Ergebnis fuer die Messzeile; *endline = 1, wenn der Parser eine Endzeile gesendet hat.
 * "ok" nur bei einer Endzeile in Erfolgsform, "fehler" bei einer Endzeile, die einen Fehler meldet (M1).
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static const char *
weather_read_answer (uint32_t start_ms, int do_get_icon, int fc, uint_fast8_t * endline)
{
    String          line;
    int             cnt = 0;
    int             rc;
    int             end;
    const char *    p;

    do
    {
        rc = weather_read_line (line, start_ms);

        if (rc == WEATHER_LINE_TIMEOUT)
        {
            return "timeout";
        }

        p = line.c_str();

        while (*p == '\r' || *p == '\n')
        {
            p++;
        }

        if (! *p)
        {
            break;                                                                                  // Leerzeile: Kopfende
        }
        cnt++;
    } while (rc == WEATHER_LINE_NL && cnt < 20);

    rc = weather_read_line (line, start_ms);

    if (rc == WEATHER_LINE_TIMEOUT)
    {
        return "timeout";
    }

    p = line.c_str();

    if (strlen (p) < 10 && rc == WEATHER_LINE_NL)                                                   // 1st data line is length of following line in Hex, e.g. "1da"
    {
        rc = weather_read_line (line, start_ms);                                                    // read next data line

        if (rc == WEATHER_LINE_TIMEOUT)
        {
            return "timeout";
        }

        p = line.c_str();
    }

    if (! *p)
    {
        return "leer";
    }

    end = fc ? parse_weather_fc (p, do_get_icon) : parse_weather (p, do_get_icon);
    *endline = (end != WEATHER_END_NONE);

    if (end == WEATHER_END_OK)
    {
        return "ok";
    }

    return (end == WEATHER_END_ERROR) ? "fehler" : "leer";
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * query_weather () - query weather for a coordinate or city
 *
 * Endet auf JEDEM Pfad mit genau einer Endzeile fuer den STM (WEATHER, WEATHER_FC, WICON, WICON_FC aus
 * dem Parser, sonst "ERROR weather <ergebnis>") und genau einer Messzeile
 * "- weather fc=<0|1> ms=<n> <ok|fehler|dns|connfail|timeout|leer>" - ohne appid, Ort, Koordinaten oder URL.
 * "fehler": Die Endzeile kam vom Parser, meldet aber einen Fehler (cod != 200, Parse Error); die Endzeile
 * bleibt dabei unveraendert, es kommt KEIN zusaetzliches "ERROR weather ..." (M1).
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static void
query_weather (char * appid, char * lon, char * lat, char * city, int do_get_icon, int fc)
{
    uint32_t        start_ms = (uint32_t) millis ();
    const char *    hostname = "api.openweathermap.org";
    String          url;
    IPAddress       ip;
    const char *    result;
    uint_fast8_t    endline = 0;
    char            logline[48];                                                                    // "- weather fc=1 ms=4294967295 connfail" = 37

    if (fc)
    {
        url = (String) "/data/2.5/forecast";                                                                    // get 3h forecast
    }
    else
    {
        url = (String) "/data/2.5/weather";
    }

    if (city)
    {
        char * pp;

        url += (String) "?q=";

        do
        {
            pp = strchr (city, ' ');

            if (pp)
            {
                *pp = '\0';
                url += city;
                url += "%20";
                city = pp + 1;
            }
        } while (pp);

        url += (String) city + "&lang=de&units=metric&APPID=" + appid;
    }
    else
    {
        url += (String) "?lon=" + lon + "&lat=" + lat + "&lang=de&units=metric&APPID=" + appid;
    }

    if (fc)
    {
        url += "&cnt=9";                                                                                    // forecast: we need only 9 records of 36 records, limit output
    }

    if (! WiFi.hostByName (hostname, ip, weather_rest_ms (start_ms)))                               // DNS mit der Restfrist
    {
        result = "dns";
    }
    else if (weather_rest_ms (start_ms) == 0)
    {
        result = "timeout";
    }
    else
    {
        openweather_client.setTimeout (weather_rest_ms (start_ms));                                 // begrenzt connect (ip, ...)

        if (openweather_client.connect (ip, 80))
        {
            debugmsg ("Connected to server");
            openweather_client.setTimeout (weather_rest_ms (start_ms));                             // begrenzt print ()
            openweather_client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + hostname + "\r\n" + "Connection: close\r\n\r\n");
            // debugmsg (String("GET ") + url + " HTTP/1.1<CR><LF>" + "Host: " + hostname + "<CR><LF>" + "Connection: close<CR><LF><CR><LF>");

            result = weather_read_answer (start_ms, do_get_icon, fc, &endline);                     // kein fester delay () mehr (L-Befund 200 ms)
            openweather_client.stop ();
        }
        else
        {
            result = "connfail";
        }
    }

    if (! endline)                                                                                  // genau eine Endzeile je Abruf (A2)
    {
        snprintf (logline, sizeof (logline), "ERROR weather %s", result);
        Serial.println (logline);
    }

    /* EINMAL formatieren, ZWEIMAL ausgeben, wie esp_heap_log () (C14/L185) */
    snprintf (logline, sizeof (logline), "- weather fc=%d ms=%lu %s",
              fc ? 1 : 0, (unsigned long) ((uint32_t) millis () - start_ms), result);
    Serial.println (logline);
    stm32_log_append (logline);
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * get_weather () - get weather for a coordinate
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
get_weather (char * appid, char * lon, char * lat)
{
    query_weather (appid, lon, lat, (char *) NULL, 0, 0);
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * get_weather () - get weather for a city
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
get_weather (char * appid, char * city)
{
    query_weather (appid, (char *) NULL, (char *) NULL, city, 0, 0);
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * get_weather_fc () - get weather for a coordinate
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
get_weather_fc (char * appid, char * lon, char * lat)
{
    query_weather (appid, lon, lat, (char *) NULL, 0, 1);
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * get_weather_fc () - get weather for a city
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
get_weather_fc (char * appid, char * city)
{
    query_weather (appid, (char *) NULL, (char *) NULL, city, 0, 1);
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * get_weather_icon () - get weather icon for a coordinate
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
get_weather_icon (char * appid, char * lon, char * lat)
{
    query_weather (appid, lon, lat, (char *) NULL, 1, 0);
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * get_weather () - get weather icon for a city
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
get_weather_icon (char * appid, char * city)
{
    query_weather (appid, (char *) NULL, (char *) NULL, city, 1, 0);
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * get_weather_icon_fc () - get forecast weather icon for a coordinate
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
get_weather_icon_fc (char * appid, char * lon, char * lat)
{
    query_weather (appid, lon, lat, (char *) NULL, 1, 1);
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * get_weather_icon_fc () - get forecast weather icon for a city
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
get_weather_icon_fc (char * appid, char * city)
{
    query_weather (appid, (char *) NULL, (char *) NULL, city, 1, 1);
}
