/* Typangleichung fuer Pruefstaende, die Firmware-Code auf dem Entwicklungsrechner laufen lassen.
 *
 * WARUM DIESE DATEI EXISTIERT -- und warum sie ein Header ist und kein Merksatz
 *
 * Ein Pruefstand auf dem Mac ist NICHT die Zielplattform (BEFUNDE.md, L256). Der
 * STM32 ist 32-bittig, der Entwicklungsrechner 64-bittig, und damit ist jeder Typ
 * ohne feste Breite eine andere Zahl:
 *
 *     unsigned long   STM32: 32 Bit        Mac/Linux 64 Bit
 *     int             beide 32 Bit         (hier harmlos)
 *     size_t          STM32: 32 Bit        Mac/Linux 64 Bit
 *     Zeiger          STM32: 32 Bit        Mac/Linux 64 Bit
 *
 * Belegt an einer Ueberlaufrechnung mit millis(): Auf dem Geraet lief sie korrekt
 * ueber den Wortrand und ergab 5049 ms; derselbe Code auf dem Pruefstand ergab
 * 4294962345 ms, weil die Differenz dort nicht ueberlief. Der Pruefstand bestaetigte
 * damit eine Rechnung, die auf dem Geraet anders ausfaellt -- die gefaehrlichste Art
 * Pruefstand, weil sein Urteil wie ein Beleg aussieht.
 *
 * ES REICHT NICHT, DAS ZU WISSEN. Deshalb steht es hier als uebersetzbarer Code:
 * Wer diesen Header einbindet, bekommt die Breiten der Zielplattform, ohne daran
 * denken zu muessen. Ein Merksatz in einer README wird beim sechsten Pruefstand
 * nicht mehr gelesen -- das Messwerkzeug der Oberflaeche ist sechsmal nachgebaut
 * worden (E19), und jedes Mal hat jemand bei null angefangen.
 */
#ifndef PRUEFSTAND_H
#define PRUEFSTAND_H

#include <stdint.h>
#include <stddef.h>

/* Die Zieltypen, unter ihren Firmware-Namen. Wer sie benutzt, rechnet wie der STM32. */
typedef uint32_t    zp_ulong;      /* entspricht unsigned long auf dem STM32 */
typedef int32_t     zp_long;
typedef uint32_t    zp_size;       /* entspricht size_t auf dem STM32 */
typedef uint32_t    zp_ptr;        /* Zeigerbreite, wenn ein Zeiger als Zahl gebraucht wird */

/* millis()-Ersatz mit der Breite der Zielplattform. Eine Differenz zweier solcher
 * Werte laeuft ueber, wie sie es auf dem Geraet tut -- genau der Fall aus L256. */
typedef uint32_t    zp_millis;

/* Gegenprobe zur Uebersetzungszeit: Stimmt eine Breite nicht, bricht der Bau ab,
 * statt eine falsche Zahl zu liefern. Ein Pruefstand, der stillschweigend das
 * Falsche misst, ist schlimmer als keiner. */
typedef char zp_breiten_pruefung[
    (sizeof(zp_ulong) == 4 && sizeof(zp_size) == 4 && sizeof(zp_millis) == 4) ? 1 : -1];

#endif /* PRUEFSTAND_H */
