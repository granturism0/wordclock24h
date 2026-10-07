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

/* DIE SCHNELLEN TYPEN -- die zweite Haelfte von L256, nachgetragen am 07.10.2026.
 *
 * Auf arm-none-eabi sind uint_fast8_t und uint_fast16_t beide `unsigned int`, also
 * 32 Bit (L290, mit dem Compiler belegt). Auf dem Mac sind sie 8 bzw. 16 Bit. Ein
 * Pruefstand, der Firmware-Code mit uint_fast8_t-Parametern auf dem Host uebersetzt,
 * kuerzt deshalb still: 256 wird 0, 288 wird 32 -- und eine Bereichspruefung, die auf
 * dem Geraet abweist, nimmt auf dem Pruefstand an. In Runde S zweimal gesehen (A3,
 * Feldzaehler in esp8266.c); beide Male hat der Agent es im eigenen Pruefstand
 * angeglichen, weil dieser Header es nicht tat.
 *
 * Opt-in, weil es die Standardnamen per Makro umbiegt: VOR dem Einbinden des
 * Firmware-Codes `#define PRUEFSTAND_SCHNELLE_TYPEN` setzen (oder -D...). Dieser
 * Header muss dann VOR dem Firmware-Code stehen -- danach greift das Makro nicht mehr.
 * Typedefs lassen sich nicht umdefinieren; ein Makro ersetzt den Namen im Text, der
 * danach kommt. */
#ifdef PRUEFSTAND_SCHNELLE_TYPEN
#define uint_fast8_t    uint32_t
#define uint_fast16_t   uint32_t
#define int_fast8_t     int32_t
#define int_fast16_t    int32_t
typedef char zp_schnelle_pruefung[(sizeof(uint_fast8_t) == 4 && sizeof(uint_fast16_t) == 4) ? 1 : -1];
#endif

#endif /* PRUEFSTAND_H */
