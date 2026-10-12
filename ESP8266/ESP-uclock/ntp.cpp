/*----------------------------------------------------------------------------------------------------------------------------------------
 * ntp.cpp - NTP functions
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
#include <WiFiUdp.h>
#include "ntp.h"
#include "base.h"

const int       NTP_PACKET_SIZE = 48;                                       // NTP time stamp is in the first 48 bytes of the message
byte            ntp_packet_buffer[NTP_PACKET_SIZE];                         // buffer to hold incoming and outgoing NTP packets

WiFiUDP         ntp_udp;
unsigned int    ntp_local_port = 2421;

/*----------------------------------------------------------------------------------------------------------------------------------------
 * Annahme einer NTP-Antwort (specs/paket-2026-10-11, design.md 10.1, Ent-13; Befund L391, C58)
 *
 * Bis ESP 3.2.31 las ntp_poll_time() packetsize Byte in ntp_packet_buffer[48] und pruefte die Groesse ERST
 * DANACH: Jedes UDP-Paket ueber 48 Byte an Port 2421 schrieb hinter den Puffer, aus dem ganzen LAN ausloesbar.
 * Und jede 48-Byte-Antwort ging als "TIME" an den STM, auch "nicht synchronisiert" (LI = 3), Kiss-of-Death
 * (Stratum 0) und ein leerer Transmit-Zeitstempel -- daraus wurde "TIME 0". Seit A62 rechnet der STM mit
 * Aeren; eine falsche Zeit ist dort nicht mehr nur ein Anzeigefehler (Ent-13: tragend).
 *
 * Angenommen wird jetzt nur eine Antwort, die ALLES erfuellt -- in dieser Reihenfolge, die ersten drei ohne
 * ein einziges Byte zu lesen:
 *
 *   absender   ntp_server_ip ist gesetzt (isSet ()) UND die Quell-IP ist genau diese. Vor der ersten Anfrage ist sie
 *              ungesetzt -- und im IPv4-Bau (ip=lm2f) ist IPAddress () dabei 0.0.0.0, gleich IPAddress (0,0,0,0) nach
 *              ip_addr_cmp. lwIP laesst Quelle 0.0.0.0 durch (DHCP, IP_ACCEPT_LINK_LAYER_ADDRESSING); ohne isSet ()
 *              ginge ein Paket mit Quelle 0.0.0.0 vor der ersten Anfrage als TIME durch, im AP-Modus dauerhaft
 *              (Review E.6). isSet () weist auch 255.255.255.255 ab: Ein solcher "Zeitserver" nimmt nie etwas an.
 *              Entscheidung des Leads zu C58. Moeglich, weil der ESP KEINEN Namen
 *              aufloest: Der STM schickt den Zeitserver als IP-Text (ntp_get_time (char *), ipstr_to_ipno),
 *              es gibt also genau eine angefragte Adresse und keinen Pool, der von einer anderen antwortet.
 *   port=<n>   Quellport == 123.
 *   laenge=<n> genau NTP_PACKET_SIZE. Sonst wird NICHT gelesen: Den ungelesenen Rest verwirft das naechste
 *              parsePacket() (UdpContext::next() gibt den pbuf frei). NICHT ntp_udp.flush() -- das ist im Core
 *              3.1.2 endPacket(), also Senden (WiFiUdp.cpp:228).
 *   li=3       Leap Indicator 3: Server nicht synchronisiert.
 *   stratum=0  Kiss-of-Death bzw. unbestimmt.
 *   transmit=0 Sekundenfeld des Transmit-Zeitstempels (Byte 40..43) ist 0 -- genau das, was als TIME ginge.
 *
 * KEINE Aeren- oder Pivot-Rechnung hier: Die macht der STM seit A62 (src/main.c). Der ESP reicht die 32-Bit-
 * Sekunden unveraendert weiter, wie bisher.
 *
 * Verworfen heisst: kein "TIME" an den STM, Zeile "- ntp verworfen #<n> <grund>" im Mitschnitt UND in
 * /api/stm32_log (Weg wie die Heap-Zeile; stm32_log_append() allein erreicht den Mitschnitt nicht, L185).
 * Gedrosselt wie "var ignoriert": die erste sofort, danach hoechstens eine je Minute; #<n> zaehlt alle. Bis
 * 3.2.31 schrieb jedes Paket falscher Groesse eine Zeile ("- wrong packet size") ungedrosselt auf die STM-UART --
 * ein Paketstrom aus dem LAN wurde so zum Zeilenstrom auf der Bruecke.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#define NTP_PORT                    123
#define NTP_REJECT_LOG_INTERVAL     60000UL                                 // msec

static IPAddress        ntp_server_ip;                                      // zuletzt angefragter Server, ungesetzt = keiner
static uint32_t         ntp_reject_cnt          = 0;
static unsigned long    ntp_reject_last         = 0;
static uint_fast8_t     ntp_reject_logged       = 0;

static void
ntp_reject (const char * reason, int value)
{
    if (ntp_reject_cnt < 0xFFFFFFFFUL)                                      // saettigend
    {
        ntp_reject_cnt++;
    }

    if (! ntp_reject_logged || (millis () - ntp_reject_last) >= NTP_REJECT_LOG_INTERVAL)
    {
        char line[56];                                                      // "- ntp verworfen #4294967295 laenge=-2147483648" = 47

        ntp_reject_logged   = 1;
        ntp_reject_last     = millis ();

        if (value >= 0)
        {
            snprintf_P (line, sizeof (line), PSTR ("- ntp verworfen #%lu %s=%d"), (unsigned long) ntp_reject_cnt, reason, value);
        }
        else
        {
            snprintf_P (line, sizeof (line), PSTR ("- ntp verworfen #%lu %s"), (unsigned long) ntp_reject_cnt, reason);
        }

        Serial.println (line);
        Serial.flush ();
        stm32_log_append (line);
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * setup ntp
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
ntp_setup (void)
{
    Serial.println("- setup UDP");
    ntp_udp.begin(ntp_local_port);
    Serial.print("- local port: ");
    Serial.println(ntp_udp.localPort());
    Serial.flush ();
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * send ntp packet
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static void
send_ntp_packet (IPAddress& address)
{
    memset(ntp_packet_buffer, 0, NTP_PACKET_SIZE);

    ntp_packet_buffer[0] = 0b11100011;                                      // LI, Version, Mode
    ntp_packet_buffer[1] = 0;                                               // Stratum, or type of clock
    ntp_packet_buffer[2] = 6;                                               // Polling Interval
    ntp_packet_buffer[3] = 0xEC;                                            // Peer Clock Precision
                                                                            // 8 bytes of zero for Root Delay & Root Dispersion
    ntp_packet_buffer[12]  = 49;
    ntp_packet_buffer[13]  = 0x4E;
    ntp_packet_buffer[14]  = 49;
    ntp_packet_buffer[15]  = 52;

    ntp_server_ip = address;                                                // E.2: nur von hier wird eine Antwort angenommen
    ntp_udp.beginPacket(address, 123);
    ntp_udp.write(ntp_packet_buffer, NTP_PACKET_SIZE);
    ntp_udp.endPacket();
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * poll ntp packet
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
ntp_poll_time (void)
{
    int     packetsize;
    int     nread;

    packetsize = ntp_udp.parsePacket();

    if (packetsize)
    {
        unsigned long   hi;
        unsigned long   lo;
        unsigned long   secsSince1900;

        if (! ntp_server_ip.isSet () || ntp_udp.remoteIP () != ntp_server_ip) // vor dem Lesen; isSet: Quelle 0.0.0.0
        {
            ntp_reject ("absender", -1);
            return;
        }

        if (ntp_udp.remotePort () != NTP_PORT)
        {
            ntp_reject ("port", (int) ntp_udp.remotePort ());
            return;
        }

        if (packetsize != NTP_PACKET_SIZE)                                  // L391: NICHT lesen, Rest verwirft parsePacket()
        {
            ntp_reject ("laenge", packetsize);
            return;
        }

        nread = ntp_udp.read(ntp_packet_buffer, NTP_PACKET_SIZE);           // hoechstens 48 Byte, nie packetsize

        if (nread != NTP_PACKET_SIZE)
        {
            ntp_reject ("laenge", nread);
            return;
        }

        if ((ntp_packet_buffer[0] >> 6) == 3)                               // LI = 3: nicht synchronisiert
        {
            ntp_reject ("li", 3);
            return;
        }

        if (ntp_packet_buffer[1] == 0)                                      // Stratum 0: Kiss-of-Death
        {
            ntp_reject ("stratum", 0);
            return;
        }

        hi = word(ntp_packet_buffer[40], ntp_packet_buffer[41]);
        lo = word(ntp_packet_buffer[42], ntp_packet_buffer[43]);
        secsSince1900 = hi << 16 | lo;

        if (secsSince1900 == 0)                                             // Transmit 0: waere "TIME 0"
        {
            ntp_reject ("transmit", 0);
            return;
        }

        Serial.print ("TIME ");
        Serial.println(secsSince1900);
        Serial.flush ();
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * get time from ntp server
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
ntp_get_time (IPAddress timeserver)
{
    Serial.println("OK time");
    Serial.flush ();

    send_ntp_packet (timeserver);
}

void
ntp_get_time (char * timeserver_str)
{
    int       ipno[4];

    if (ipstr_to_ipno (ipno, timeserver_str))
    {
        IPAddress timeserver (ipno[0], ipno[1], ipno[2], ipno[3]);
        ntp_get_time (timeserver);
    }
}

void
ntp_get_time (void)
{
    IPAddress timeserver (129, 6, 15, 28);                      // time-a-g.nist.gov NTP server

    ntp_get_time (timeserver);
}
