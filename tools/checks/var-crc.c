/* Referenzrechnung fuer die Bruecken-Pruefsumme (A32, specs/bruecke-wiederholung).
 *
 * WARUM ES DIESE DATEI GIBT
 *
 * Die Pruefsumme wird an ZWEI Stellen gebildet -- im STM (C) und im ESP (C++).
 * Zwei Umsetzungen derselben Rechnung sind zwei Wahrheiten, und sie laufen
 * auseinander, sobald eine davon angefasst wird. Diese Datei ist die dritte,
 * unabhaengige: Sie erzeugt die Pruefvektoren, gegen die beide anderen antreten.
 *
 * Sie entsteht BEWUSST VOR den beiden Umsetzungen. Wer die Vektoren aus einer
 * bestehenden Umsetzung ableitet, prueft nur, ob sie mit sich selbst
 * uebereinstimmt -- das ist der Fall, in dem beide Seiten denselben Fehler haben
 * und die Pruefung ihn bestaetigt.
 *
 * Rechenweg nach design.md 6.1: Fletcher-artig, Laenge als Startwert, damit eine
 * Laengenaenderung -- also der belegte Burst-Verlust aus L141/L188 -- sich
 * auswirkt. Gerechnet wird ueber die Nutzlast OHNE "var " und OHNE die
 * Pruefsumme selbst.
 *
 *   cc -I tools/checks -o /tmp/var-crc tools/checks/var-crc.c && /tmp/var-crc
 *
 * ZUR TYPANGLEICHUNG (L256, pruefstand.h)
 *
 * Dieser Pruefstand band pruefstand.h bis zum 05.10.2026 NICHT ein -- und war
 * zugleich das einzige Beispiel in Guardrail-Stufe S13. Die Breitenpruefung des
 * Headers, die bei falscher Zielbreite den Bau abbricht, lief damit in keinem
 * einzigen Durchlauf mit. Gemeldet hat es der Umsetzer von A41, dem es beim
 * Einbinden fuer seinen eigenen Pruefstand auffiel -- nicht die Stufe.
 *
 * Die Rechnung hier ist von der Wortbreite UNABHAENGIG: Sie laeuft ueber uint8_t
 * und uint16_t, und das size_t zaehlt nur Zeichen einer kurzen Zeichenkette. Der
 * Header wird trotzdem eingebunden, denn genau das ist sein Zweck -- er prueft die
 * Breiten zur Uebersetzungszeit, und ein Pruefstand, der ihn auslaesst, nimmt die
 * Pruefung fuer alle anderen mit heraus.
 */
#include "pruefstand.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

uint16_t
var_crc (const char * payload)
{
    uint8_t     sum1;
    uint8_t     sum2 = 0;
    size_t      len  = strlen (payload);
    size_t      i;

    sum1 = (uint8_t) len;

    for (i = 0; i < len; i++)
    {
        sum1 = (uint8_t) (sum1 + (uint8_t) payload[i]);
        sum2 = (uint8_t) (sum2 + sum1);
    }

    return (uint16_t) ((sum2 << 8) | sum1);
}

int
main (void)
{
    /* Die Vektoren sind nicht beliebig gewaehlt:
     *   OT0002   der Fall aus L205 in seiner richtigen Form
     *   OT0      derselbe, auf die Laenge verkuerzt, die 14 erzeugte
     *   OT000    die Zwischenstufe, die still 0 ergab
     *   S03...   ein Zeichenkettenkommando -- hier trifft die Marke auf freien Text
     *   N0a0f    das haeufigste Kommando ueberhaupt (Zahlenvariable)
     *   (leer)   Randfall, Laenge 0
     */
    static const char * const vektoren[] = {
        "OT0002", "OT000", "OT0", "N0a0f", "S03meinhost", "", "DC00ff800000ff", "T0120261004213000"
    };
    size_t i;

    printf ("Pruefvektoren fuer die Bruecken-Pruefsumme\n");
    printf ("erzeugt aus tools/checks/var-crc.c, unabhaengig von beiden Umsetzungen\n\n");
    printf ("  %-22s %-5s %s\n", "Nutzlast", "Laenge", "Pruefsumme");
    printf ("  %-22s %-5s %s\n", "----------------------", "-----", "----------");

    for (i = 0; i < sizeof (vektoren) / sizeof (vektoren[0]); i++)
    {
        printf ("  %-22s %-5zu *%04x\n",
                vektoren[i][0] ? vektoren[i] : "(leer)",
                strlen (vektoren[i]),
                var_crc (vektoren[i]));
    }

    printf ("\n  Gegenprobe: die beiden Faelle aus L205 muessen sich unterscheiden --\n");
    printf ("  sonst haette die Pruefsumme den Befund nicht gefunden, gegen den sie gebaut ist.\n");
    printf ("    OT0002 -> *%04x\n    OT0    -> *%04x\n",
            var_crc ("OT0002"), var_crc ("OT0"));
    printf ("    %s\n", var_crc ("OT0002") != var_crc ("OT0") ? "verschieden -- gut" : "GLEICH -- die Pruefsumme taugt nicht");

    return 0;
}
