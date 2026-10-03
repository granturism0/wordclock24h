/*----------------------------------------------------------------------------------------------------------------------------------------
 * httpclient.cpp - http client
 *
 * Copyright (c) 2018-2025 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#include <ESP8266WiFi.h>
#include <strings.h>
#include "base.h"

static WiFiClient      client;

/*----------------------------------------------------------------------------------------------------------------------------------------
 * Warten auf Daten der Gegenstelle - mit Abbruchbedingung (BEFUNDE.md L152)
 *
 * Vorher stand an beiden Lesestellen "while (! client.available()) { ; }" ohne jede
 * Abbruchbedingung. Dass das NICHT in einem Watchdog-Reset endet, macht es schlimmer
 * statt besser: WiFiClient::available () ruft bei leerem Puffer optimistic_yield (100)
 * auf (WiFiClient.cpp:254), und optimistic_yield () fuehrt aus dem CONT-Kontext heraus
 * ein echtes yield () aus (core_esp8266_main.cpp:200). Beide Watchdogs werden also
 * weiter bedient - die Schleife laeuft nicht bis zum Reset, sondern UNBEGRENZT. loop ()
 * kommt nie wieder, die Bruecke zum STM steht still, und das Geraet startet nicht
 * einmal neu. Ein zusaetzliches yield () haette daran nichts geaendert; noetig ist eine
 * Zeitgrenze mit definiertem Rueckgabewert.
 *
 * client.connected () ist die richtige zweite Bedingung und nicht zu scharf: Der Core
 * liefert true, solange noch gepufferte Daten anstehen, auch wenn die Gegenstelle
 * bereits geschlossen hat (WiFiClient.cpp:332). Erst wenn nichts mehr da ist UND die
 * Verbindung weg ist, bricht die Schleife ab.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#define HTTPCLIENT_READ_TIMEOUT     5000                        // msec, Vorbild: READ_BODY_TIMEOUT in http.cpp

static bool
httpclient_wait_for_data (void)
{
    unsigned long   start_millis = millis ();

    while (! client.available ())
    {
        if (! client.connected ())
        {
            return false;                                       // Gegenstelle weg und Puffer leer
        }

        if ((millis () - start_millis) >= HTTPCLIENT_READ_TIMEOUT)
        {
            return false;                                       // Zeitgrenze, Differenzbildung ist ueberlaufsicher
        }

        delay (1);
    }

    return true;
}

int
httpclient_read_header (int * lenp)
{
    char    linebuf[256];
    char *  p;
    int     cnt = 0;
    int     ch;
    int     len = 0;
    int     errorcode = 0;

    while (client.available())                           // skip http header
    {
        ch = client.read();

        if (ch == '\n')
        {
            if (cnt == 0)
            {
                break;
            }

            linebuf[cnt] = '\0';

            if (! mystrnicmp (linebuf, "HTTP", 4))
            {
                p = strchr (linebuf, ' ');

                if (p)
                {
                    while (*p == ' ')
                    {
                        p++;
                    }
                    errorcode = atoi (p);
                }
            }
            else if (! mystrnicmp (linebuf, "Content-Length: ", 16))
            {
                p = strchr (linebuf, ' ');

                if (p)
                {
                    while (*p == ' ')
                    {
                        p++;
                    }

                    len = atoi (p);
                }
            }

            cnt = 0;
        }
        else if (ch != '\r')
        {
            if (cnt < 256 - 1)
            {
                linebuf[cnt++] = ch;
            }
        }
    }

    *lenp = len;
    return errorcode;
}

int
httpclient (const char * host, const char * path, const char * file)
{
    const int       port = 80;
    int             errorcode;
    int             len;

    if (! client.connect(host, port))
    {
        return -1;
    }

    client.print (String("GET ") + "/" + path + "/" + file + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

    if (! httpclient_wait_for_data ())                                              // erstes Antwortbyte
    {
        client.stop();                                                              // auch bei Reset der Gegenstelle: nicht 5 s leer drehen
        return -1;
    }

    errorcode = httpclient_read_header (&len);

    if (errorcode != 200)                                                               // webserver errorcode != 200 (OK)
    {
        client.stop ();
        return -1;
    }

    return len;                                                                         // length of content to be read
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * httpclient_read () - ein Zeichen lesen
 *
 * RUECKGABEWERT -1 HAT EINE NEUE BEDEUTUNG. Frueher kam er ausschliesslich bei
 * leerem Rest (*lenp <= 0), also nie innerhalb einer "while (len > 0)"-Schleife;
 * jeder Aufrufer durfte ihn deshalb ignorieren. Jetzt meldet er zusaetzlich einen
 * FEHLSCHLAG: Zeitgrenze erreicht oder Gegenstelle weg.
 *
 * *lenp bleibt in diesem Fall ABSICHTLICH unveraendert. Wuerde es auf 0 gesetzt,
 * saehe ein abgebrochener Abruf wie ein vollstaendiger aus - in
 * stm32_flash_download_image () entscheidet genau "len > 0" darueber, ob eine halb
 * geladene Firmware verworfen wird (DIR-010).
 *
 * Daraus folgt: JEDE Schleife der Form "while (len > 0) { ch = httpclient_read
 * (&len); ... }" MUSS bei ch < 0 abbrechen, sonst dreht sie endlos. Alle
 * Aufrufstellen in http.cpp sind entsprechend nachgezogen.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
int
httpclient_read (int * lenp)
{
    int     len = *lenp;
    int     ch = -1;

    if (len > 0)
    {
        if (! httpclient_wait_for_data ())
        {
            return -1;                                  // Zeitgrenze oder Abbruch, *lenp bleibt stehen
        }

        ch = client.read();

        if (ch < 0)
        {
            return -1;                                  // available () hat gemeldet, read () liefert trotzdem nichts
        }

        len--;
        *lenp = len;
    }
    return ch;
}

int
httpclient_read_line (unsigned char * bufp, int buflen, int * lenp)
{
    int     bufpos;
    int     ch;
    int     len = *lenp;

    bufpos = 0;

    while (len > 0 && bufpos < buflen)
    {
        if (! httpclient_wait_for_data ())
        {
            *bufp = '\0';
            *lenp = len;                                // Rest stehen lassen: der Aufrufer erkennt den Fehlschlag daran
            return -1;
        }

        ch = client.read();

        if (ch < 0)
        {
            *bufp = '\0';
            *lenp = len;
            return -1;
        }

        *bufp++ = ch;
        bufpos++;

        len--;

        if (ch == '\n')
        {
            break;
        }
    }

    *bufp = '\0';
    *lenp = len;
    return bufpos;
}

void
httpclient_stop (void)
{
    client.stop ();
}
