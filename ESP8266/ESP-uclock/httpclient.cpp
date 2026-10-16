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
#include "httpclient.h"                                         // R1b L7: httpclient_stop () steht weiter unten

static WiFiClient      client;
static uint16_t        httpclient_connects = 0;                // E3 I-1 (C65): erfolgreiche connect () seit dem Start, saettigend

/*----------------------------------------------------------------------------------------------------------------------------------------
 * Warten auf Daten der Gegenstelle - mit Abbruchbedingung (BEFUNDE.md L152, L173)
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
 * HIER STAND EINE FALSCHE BEGRUENDUNG, und genau sie hat L173 verursacht. Sie lautete:
 * "client.connected () liefert true, solange noch gepufferte Daten anstehen, auch wenn
 * die Gegenstelle bereits geschlossen hat (WiFiClient.cpp:332)". Das stimmt nicht.
 * WiFiClient::connected () fragt ZUERST ClientContext::state () ab und kehrt bei CLOSED
 * sofort mit 0 zurueck (WiFiClient.cpp:329) - und ClientContext::state () meldet CLOSED
 * auch fuer CLOSE_WAIT und CLOSING (ClientContext.h:365). Das "|| available ()" in Zeile
 * 332 wird also in genau dem Fall nie erreicht, fuer den es hier zitiert wurde.
 *
 * Am Geraet gemessen (04.10.2026): Der Abbruch traf reproduzierbar nach exakt 536
 * empfangenen Byte, also nach genau EINEM TCP-Segment (TCP_MSS = 536 in der Bauvariante
 * ip=lm2f). Gegen zwei Server mit verschiedenen Kopflaengen dieselbe Summe: 248 + 288
 * und 354 + 182. Der Abbruch kam SOFORT - die abgeschnittenen Abrufe waren genauso
 * schnell wie die vollstaendigen, die Zeitgrenze war also nicht beteiligt. Betroffen war
 * der Legacy-Pfad genauso wie /api/update_status, weil beide dieselbe Leseschleife
 * benutzen.
 *
 * Dass die fehlenden Byte trotzdem ankommen, ist ebenfalls belegt: Bis ESP 3.2.16
 * wartete genau diese Stelle unbegrenzt und lieferte die Datei IMMER vollstaendig.
 * VERALTET war dabei nur die erste Rueckgabe von available (), NICHT das Ende (L353,
 * C53/L356): available () erhebt den Puffer ZUERST und gibt erst DANACH per
 * optimistic_yield (100) ab (WiFiClient.cpp:246-257). In dieser Abgabe stellt der Treiber
 * die restlichen Segmente samt FIN zu, zurueck kommt die alte 0. Das Ende selbst ist
 * endgueltig: Das FIN folgt den Daten in Reihenfolge, und der Puffer bleibt beim FIN
 * erhalten (ClientContext.h:594-615). Was available () NACH connected () == 0 meldet,
 * ist vollstaendig - es kommt nichts mehr nach.
 *
 * Deshalb beendet connected () == false die Schleife nicht mehr sofort, sondern eroeffnet
 * ein NACHLAUFFENSTER: Es wird weiter auf Daten geprueft, bis das Budget aufgebraucht
 * ist. Das Budget gilt FUER DEN GANZEN ABRUF und nicht je Zeichen - sonst koennten 432
 * Restbyte im schlimmsten Fall 432 Fenster kosten. Beim regulaeren Ende eines Abrufs
 * kostet es nichts, weil der Aufrufer dann bei len == 0 aufhoert und gar nicht mehr
 * wartet; bezahlt wird es nur von Abrufen, die tatsaechlich abreissen. Noetig waere nur
 * der ERSTE Schritt des Fensters, die erneute Abfrage von available (); der Rest des
 * Budgets ist Reserve und kostet nur bei einer Verbindung, die ohne Rest endet.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#define HTTPCLIENT_READ_TIMEOUT     5000                        // msec, Vorbild: READ_BODY_TIMEOUT in http.cpp
#define HTTPCLIENT_PEER_GONE_GRACE   300                        // msec Nachlauf JE ABRUF, wenn connected () schon false meldet

static unsigned int     httpclient_grace_left = 0;              // Rest des Nachlaufbudgets, von httpclient () je Abruf gesetzt

static bool
httpclient_wait_for_data (void)
{
    unsigned long   start_millis = millis ();

    while (! client.available ())
    {
        if ((millis () - start_millis) >= HTTPCLIENT_READ_TIMEOUT)
        {
            return false;                                       // Zeitgrenze, Differenzbildung ist ueberlaufsicher
        }

        if (! client.connected ())
        {
            if (httpclient_grace_left == 0)
            {
                return false;                                   // Nachlauf aufgebraucht: jetzt kommt wirklich nichts mehr
            }

            httpclient_grace_left--;                            // ein Schritt je delay (1) unten
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
    int     complete = 0;
    unsigned long   start_millis = millis ();                   // A3: Zeitgrenze fuer den GANZEN Kopf

    // C9k/L172: ueber den Helfer aus L152 warten statt nur zu lesen, was schon da ist - ein Kopf
    // in zwei Segmenten galt sonst als fertig, und ohne Content-Length kam len = 0 zurueck
    while (httpclient_wait_for_data ())                  // skip http header
    {
        // A3 (Review F.6): httpclient_wait_for_data () begrenzt nur das Warten auf das NAECHSTE Zeichen.
        // Ein Kopf ohne Leerzeile - endlos oder tropfend - hielte loop () sonst unbegrenzt an. Ueber den
        // ganzen Kopf gilt deshalb dieselbe Zeitgrenze; laengster Fall knapp zwei Zeitgrenzen.
        if ((millis () - start_millis) >= HTTPCLIENT_READ_TIMEOUT)
        {
            break;                                      // complete bleibt 0: Fehlschlag
        }

        ch = client.read();

        if (ch < 0)
        {
            break;
        }

        if (ch == '\n')
        {
            if (cnt == 0)
            {
                complete = 1;                           // Leerzeile: Kopf vollstaendig
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
    return complete ? errorcode : -1;                   // abgerissener Kopf ist ein Fehlschlag, kein 200
}

/* Paket 2026-10-15 R1b (L7b, L7c = C75): JEDER Rueckweg mit einem Wert <= 0 laeuft ueber httpclient_stop () und gibt
 * damit den ClientContext frei. Kein Aufrufer muss das wissen: Alle pruefen auf len > 0 und schliessen nur in diesem
 * Zweig selbst; bei <= 0 geben sie auf. Bis R1 blieben dabei offen bzw. gebunden: der Rumpf einer Fehlerseite
 * (Status != 200, etwa 404), eine Antwort nach der Zeitgrenze und ein 200 ohne Rumpf (len 0, C75). Der Rueckgabewert
 * bleibt in allen Faellen derselbe wie bisher.
 */
int
httpclient (const char * host, const char * path, const char * file)
{
    const int       port = 80;
    int             errorcode;
    int             len;

    tcp_tw_limit ();                                                                // E4 Teil 5 (C65): TIME_WAIT <= 2, unmittelbar VOR connect ()

    if (! client.connect(host, port))
    {
        httpclient_stop ();                                                         // R1b L7: scheitert schon der DNS, laesst connect () einen alten Kontext stehen
        return -1;
    }

    /* E3 I-1 (C65, design.md 18.2): Messpunkt "client" nach erfolgreichem connect (). Hier laufen ALLE
     * ausgehenden Abrufe ueber httpclient () durch - die der Anfragen, der Auftrag update_cache_poll (),
     * die Legacy-Seiten und der STM-Flash-Download. Der Wetterabruf hat einen eigenen Client und zaehlt nicht.
     */
    heap_low_note (HEAP_LOW_PATH_CLIENT);

    if (httpclient_connects < 0xFFFF)
    {
        httpclient_connects++;
    }

    httpclient_grace_left = HTTPCLIENT_PEER_GONE_GRACE;                             // Nachlaufbudget gilt je Abruf (L173)

    client.print (String("GET ") + "/" + path + "/" + file + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

    if (! httpclient_wait_for_data ())                                              // erstes Antwortbyte
    {
        httpclient_stop ();                                                         // auch bei Reset der Gegenstelle: nicht 5 s leer drehen; R1b L7b: freigeben
        return -1;
    }

    errorcode = httpclient_read_header (&len);

    if (errorcode != 200)                                                               // webserver errorcode != 200 (OK)
    {
        httpclient_stop ();                                                             // R1b L7b: Rumpf der Fehlerseite (etwa 404) nicht liegen lassen
        return -1;
    }

    if (len <= 0)                                                                       // R1b L7c (C75): 200 ohne Rumpf - nichts zu lesen
    {
        httpclient_stop ();
        return len;
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

/*----------------------------------------------------------------------------------------------------------------------------------------
 * httpclient_stop () - Verbindung schliessen UND den ClientContext freigeben (Paket 2026-10-15, R1b, L7a; C71)
 *
 * WiFiClient::stop () ruft nur ClientContext::close (): Der PCB wird geschlossen, die schon empfangene, aber nicht
 * gelesene Antwort bleibt als pbuf-Kette in _rx_buf stehen (ClientContext.h:73-92, WiFiClient.cpp:316-325). Frei wird
 * sie erst, wenn der Refcount auf 0 faellt: unref () ruft dann discard_received (), close () und delete
 * (ClientContext.h:115-127, :302-314). Der statische client hier hielt seinen Kontext aber bis zum NAECHSTEN connect ()
 * (WiFiClient.cpp:145-151). Jede Pruefung, die den Rumpf nicht liest - update_status mit wc12h-icon.txt und
 * wc12h-weather.txt, die Pruefung auf app.js.gz, der Zweig zu-gross in update_cache_write () -, band so die ganze
 * Antwort bis zum naechsten Abruf: am Geraet 1'704 Byte in drei pbufs nach jedem kalten update_status (BEFUNDE L468,
 * Ursache von C71).
 *
 * Deshalb nach stop () die Zuweisung eines leeren WiFiClient. operator= ruft unref () auf den alten Kontext
 * (WiFiClient.cpp:117-128); er gehoert nur diesem client (Refcount 1, keine Kopie), faellt auf 0 und wird samt _rx_buf
 * freigegeben. Danach ist der Zeiger im client NULL: Kein Code kann den alten Kontext mehr erreichen, available (),
 * read () und connected () liefern ohne Kontext 0 bzw. -1, und der naechste connect () legt einen frischen an (rund
 * 56 Byte), wie bisher beim ersten Abruf nach dem Start. Der leere WiFiClient bringt _timeout 5000 mit
 * (WiFiClient.cpp:76-81) - dieselbe Vorgabe, die client hier immer hatte, denn niemand setzt eine eigene.
 * Ein zweiter Aufruf ist harmlos: stop () und operator= tun ohne Kontext nichts.
 *
 * Der Wetter-Client (openweather_client, weather.cpp) ist ein eigener WiFiClient und bleibt unberuehrt.
 * RAM: 0 Byte statisch.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
httpclient_stop (void)
{
    client.stop ();
    client = WiFiClient ();                                     // R1b L7a: Kontext samt ungelesener Antwort freigeben
}

uint16_t
httpclient_connects_get (void)
{
    return httpclient_connects;
}
