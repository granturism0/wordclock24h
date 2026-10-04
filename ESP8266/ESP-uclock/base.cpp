/*----------------------------------------------------------------------------------------------------------------------------------------
 * base.cpp - base functions
 *
 * Copyright (c) 2016-2025 Frank Meyer - frank(at)uclock.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#include <stdint.h>
#include "base.h"

 /*--------------------------------------------------------------------------------------------------------------------------------------
 * get day of week (0=Sunday, 1=Monday, ... 6=Saturday)
 *
 *  day         - day of month
 *  month       - month beginning with 1
 *  year        - greater than 2000
 *
 *  example:    int rtc = dayofweek (tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
int
dayofweek (int d, int m, int y)
{
   return (d += m < 3 ? y-- : y - 2 , 23 * m / 9 + d + 4 + y / 4 - y / 100 + y / 400) % 7;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * hex to integer
 *
 * Die Schleifenbedingung prueft buf[i] und NICHT *buf -- Befund L232, und das ist kein
 * Schoenheitsfehler: *buf ist immer buf[0], der Zeiger wandert nie. Die Abbruchbedingung
 * waere damit nach dem ersten Zeichen bedeutungslos, und bei einer Zeichenkette kuerzer
 * als max_digits laese die Funktion ueber das Zeilenende hinaus in den Rest des Puffers.
 *
 * Die Folge ist schwerer als ein Absturz: Ein auf der Bruecke verlorenes Zeichen erzeugt
 * dann keinen FEHLENDEN Wert, sondern einen gueltig aussehenden FALSCHEN -- rechtsbuendig
 * aufgefuellt mit dem, was zufaellig dahinter steht. Nachgerechnet am alten Stand:
 * Das Typfeld von "OT<idx><typ>" ist zwei Zeichen breit. Fehlt das letzte Zeichen, bleibt
 * "OT000" -> Typ 0 und "OT002" -> Typ 32: Die 2 rutscht ins obere Nibble, das untere wird
 * mit dem gefuellt, was hinter dem Zeilenende steht -- hier der Terminator, der ueber den
 * else-Zweig als 0 gilt. Steht dort kein Terminator, sondern Resttext aus dem Puffer, ist
 * der Wert beliebig. Das ist der Mechanismus hinter L205, wo overlay[0].type als 14 statt
 * 2 ankam -- und weil 14 gueltig aussieht, fiel es nirgends auf.
 *
 * Also bitte nicht "aufraeumen" und wieder auf *buf zurueckstellen. Betroffen ist jede
 * Hexzahl, die der ESP von der Bruecke liest.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint16_t
htoi (char * buf, uint8_t max_digits)
{
    uint8_t     i;
    uint8_t     x;
    uint16_t    sum = 0;

    for (i = 0; i < max_digits && buf[i]; i++)
    {
        x = buf[i];

        if (x >= '0' && x <= '9')
        {
            x -= '0';
        }
        else if (x >= 'A' && x <= 'F')
        {
            x -= 'A' - 10;
        }
        else if (x >= 'a' && x <= 'f')
        {
            x -= 'a' - 10;
        }
        else
        {
            x = 0;
        }
        sum <<= 4;
        sum += x;
    }

    return (sum);
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * ipstr_to_ipno() - convert an ipaddress string into numbers
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
bool
ipstr_to_ipno (int * ipno, const char * address)
{
    uint16_t sum = 0;
    uint8_t  dots = 0;

    while (*address)
    {
        char ch = *address++;

        if (ch >= '0' && ch <= '9')
        {
            sum = sum * 10 + (ch - '0');

            if (sum > 255)
            {
                return false;
            }
        }
        else if (ch == '.')
        {
            if (dots == 3)
            {
                return false;
            }

            ipno[dots++] = sum;
            sum = 0;
        }
        else
        {
            return false;                                                   // invalid character
        }
    }

    if (dots != 3)
    {
        return false;
    }

    ipno[3] = sum;
    return true;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * trim string
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
trim (char * bufp)
{
    int len = strlen (bufp);

    if (len > 0)
    {
        char * pp = bufp + len - 1;

        while (len > 0 && (*pp == ' ' || *pp == '\t'))
        {
            *pp-- = '\0';
            len--;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * substitute characters
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
strsubst (char * s, int oldchar, int newchar)
{
    while (*s)
    {
        if (*s == oldchar)
        {
            *s = newchar;
        }

        s++;
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * mystrnicmp - Arduino does not support strnicmp() nor strncasecmp()
 * This function works only correct for 7Bit ASCII.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
int
mystrnicmp (const char * s1, const char * s2, int n)
{
    int ch1;
    int ch2;

    while (n && (*s1 || *s2))
    {
        if (*s1 >= 'a' && *s1 <= 'z')
        {
            ch1 = *s1 - 'a' + 'A';
        }
        else
        {
            ch1 = *s1;
        }

        if (*s2 >= 'a' && *s2 <= 'z')
        {
            ch2 = *s2 - 'a' + 'A';
        }
        else
        {
            ch2 = *s2;
        }

        if (ch1 != ch2)
        {
            return ch1 - ch2;
        }

        if (*s1)
        {
            s1++;
        }

        if (*s2)
        {
            s2++;
        }

        n--;
    }

    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * convert_utf8_to_iso8859 () - convert string to iso8859-1
 *
 * Befund L236: Ein Fuehrungsbyte als LETZTES Byte vor dem Terminator liess die alte Fassung
 * ueber die Pufferkante lesen. Der Ablauf war: *s == 0xC3 -> s++ (zeigt jetzt auf den
 * Terminator) -> *t++ = *s++ + 0x40 liest den TERMINATOR als Nutzbyte (schreibt 0x00 + 0x40,
 * also '@') und schiebt s HINTER den Terminator. Die Schleife while (*s) las danach den Rest
 * des Puffers und alles, was dahinter im Speicher lag, bis zufaellig eine Null kam.
 * Das SCHREIBEN war durch len == MAX_ISO8BUFLEN - 1 begrenzt, das LESEN nicht.
 * Dasselbe galt fuer 0xC2, und der Zweig *s > 0xC0 mit s += 2 konnte den Terminator
 * ebenfalls ueberspringen.
 *
 * Warum das mehr als ein Schoenheitsfehler ist: Die einzigen beiden Aufrufstellen stehen in
 * weather.cpp:151 und :217 und reichen die Wetterbeschreibung einer FREMDEN Website herein.
 * parse_json() schneidet sie bei MAX_LEN_DESCRIPTION - 1 = 31 Byte hart ab, ohne auf
 * Zeichengrenzen zu achten (weather.cpp:87-93). Faellt der Schnitt auf ein Fuehrungsbyte,
 * ist der Fall da. Damit loest eine fremde Antwort einen Lesezugriff ausserhalb des Puffers
 * aus -- es braucht nur eine Beschreibung, die an der falschen Stelle endet.
 *
 * Nachgerechnet mit einer Attrappe: Eingabe so gelegt, dass ihr Terminator genau auf der
 * letzten Byteposition einer Speicherseite liegt, dahinter eine Schutzseite. Die Eingaben
 * "ab\xC3", "ab\xC2", "ab\xE2" und "\xC3" erzeugten mit der ALTEN Fassung jeweils SIGBUS,
 * mit dieser Fassung keinen einzigen. Gueltige Eingaben ("clear sky", "bew\xC3\xB6lkt",
 * "\xC2\xB0" "C", "" ) liefern in beiden Fassungen Byte fuer Byte dasselbe Ergebnis.
 * Bemerkenswert dabei: Im Fall 0xC2 sieht die AUSGABE harmlos aus (die 0x00 landet im Ziel
 * und beendet die Zeichenkette dort) -- der Ueberlauf passiert trotzdem. Wer nur die Ausgabe
 * prueft, findet diesen Fall nicht.
 *
 * Die Korrektur prueft nach jedem s++ auf den Terminator und bricht ab. Das halbe Zeichen
 * faellt weg, und das ist gewollt: Ein abgeschnittenes Zeichen hat keine Entsprechung in
 * ISO-8859-1, ein '@' an seiner Stelle waere eine erfundene.
 *
 * BEWUSST NICHT geaendert: Der Zweig *s > 0xC0 springt weiterhin nur 2 Byte weit, obwohl ein
 * Drei-Byte-Zeichen 3 braucht; die Folgebytes werden dann als Literal kopiert. Das ist seit
 * jeher so, bleibt durch len begrenzt und ist eine Darstellungsfrage, kein Speicherfehler.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MAX_ISO8BUFLEN    128
unsigned char *
convert_utf8_to_iso8859 (const unsigned char * buf)
{
    static unsigned char    iso8buf[MAX_ISO8BUFLEN];
    const unsigned char *   s;                          // source ptr
    unsigned char *         t;                          // target ptr
    int                     len = 0;

    s = buf;
    t = iso8buf;

    while (*s)
    {
        if (*s == 0xC3)
        {
            s++;

            if (! *s)                                   // L236: Fuehrungsbyte war das letzte Byte --
            {                                           // der Terminator ist KEIN Nutzbyte
                break;
            }

            *t++ = *s++ + 0x40;
            len++;
        }
        else if (*s == 0xC2)
        {
            s++;

            if (! *s)                                   // L236: dito
            {
                break;
            }

            *t++ = *s++;
            len++;
        }
        else if (*s >0xC0)                              // unknown codepages
        {
            s++;

            if (! *s)                                   // L236: das frueher unbedingte s += 2
            {                                           // konnte den Terminator ueberspringen
                break;
            }

            s++;
        }
        else
        {
            *t++ = *s++;
            len++;
        }

        if (len == MAX_ISO8BUFLEN - 1)
        {
            break;
        }
    }

    *t = '\0';                                          // terminate target

    return (iso8buf);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * send status message to WordClock
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
statusmsg (const char * msg, const char * status)
{
    Serial.print (msg);
    Serial.print (" ");
    Serial.println (status);
    Serial.flush ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * print debug message via UART connected to WordClock
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
debugmsg (String msg)
{
    Serial.print ("- ");
    Serial.println (msg);
    Serial.flush ();
}

void
debugmsg (const char * msg)
{
    if (msg && * msg)
    {
        Serial.print ("- ");
        Serial.println (msg);
        Serial.flush ();
    }
}

void
debugmsg (const char * str, const char * msg)
{
    if (str && *str && msg && * msg)
    {
        Serial.print ("- ");
        Serial.print (str);
        Serial.print (": ");
        Serial.println (msg);
        Serial.flush ();
    }
}
