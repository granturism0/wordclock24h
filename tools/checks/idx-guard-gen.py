#!/usr/bin/env python3
# Erzeugt einen Pruefstand aus dem ECHTEN Quelltext: die drei Handler, esp8266_idx_ok ()
# und die fuenf Setter werden woertlich aus src/main.c bzw. src/display/display.c
# herausgeschnitten, nicht nachgebaut.
#
# WARUM DAS DER PUNKT IST
#
# Ein Pruefstand, der die Logik NACHBAUT, prueft seinen eigenen Nachbau. Dieser hier
# schneidet die Funktionen woertlich heraus -- wer den Guard spaeter entfernt oder
# die Grenze aendert, bricht ihn, und zwar beim naechsten Guardrail-Lauf.
#
# Er traegt die Befunde A43/L286 und A44/L289: fuenf Setter und drei Handler schrieben
# einen Index aus dem LAN (0..255) ungeprueft in Arrays mit 3 bis 13 Plaetzen. Gemessen
# ohne die Guards: 1241 Verletzungen, hoechster Offset 5658 Byte hinter dem Struct.
# Mit den Guards: 0 -- und keine gueltige Zeile faellt heraus.
#
#     python3 tools/checks/idx-guard-gen.py            erzeugt den Pruefstand
#     python3 tools/checks/idx-guard-gen.py --strip-guard   dasselbe OHNE die Guards
#
# Die zweite Form ist der Fehlschlag-Nachweis nach DIR-014 und gehoert zu jedem Lauf:
# Ein Pruefstand, der nur gruen gesehen wurde, hat nichts gezeigt.
import sys, re

# Wurzel relativ bestimmen -- ein absoluter Pfad im Repo schlaegt in S9 an und
# bricht, sobald jemand anders das Projekt auscheckt.
import subprocess
ROOT = subprocess.run(["git", "rev-parse", "--show-toplevel"],
                      capture_output=True, text=True).stdout.strip() + "/"
STRIP_GUARD = "--strip-guard" in sys.argv          # fuer den Fehlschlag-Nachweis (DIR-014)

def load(p):
    return open(ROOT + p, "rb").read().replace(b"\r\n", b"\n").decode("latin-1")

def cut(src, header, what):
    """Schneidet eine Funktion ab ihrer Signaturzeile bis zur schliessenden Klammer in Spalte 0."""
    i = src.find(header)
    if i < 0:
        sys.exit("nicht gefunden: " + what)
    if src.find(header, i + 1) >= 0:
        sys.exit("mehrdeutig: " + what)
    i = src.rfind("\n", 0, i) + 1                 # Rueckgabetypzeile davor mitnehmen
    i = src.rfind("\n", 0, i - 1) + 1
    j = src.find("\n{\n", i)
    k = src.find("\n}\n", j)
    if j < 0 or k < 0:
        sys.exit("Rumpf nicht abgrenzbar: " + what)
    return src[i:k+3]

main = load("src/main.c")
disp = load("src/display/display.c")
base = load("src/base/base.c")

idx_ok   = cut(main, "esp8266_idx_ok (uint_fast8_t idx, uint_fast8_t limit, const char * was)", "esp8266_idx_ok")
h_anim   = cut(main, "schedule_esp8266_animation_variable (char * parameters)", "Handler anim")
h_col    = cut(main, "schedule_esp8266_color_animation_variable (char * parameters)", "Handler colanim")
h_amb    = cut(main, "schedule_esp8266_ambilight_mode (char * parameters)", "Handler ambimode")
htoi     = cut(base, "htoi (char * buf, uint8_t max_digits)", "htoi")

setters = "".join(cut(disp, sig, sig) for sig in [
    "display_set_animation_deceleration (uint_fast8_t idx, uint_fast8_t deceleration)",
    "display_set_animation_flags (uint_fast8_t idx, uint_fast8_t flags)",
    "display_set_color_animation_deceleration (uint_fast8_t idx, uint_fast8_t deceleration)",
    "display_set_ambilight_mode_deceleration (uint_fast8_t idx, uint_fast8_t deceleration)",
    "display_set_ambilight_mode_flags (uint_fast8_t idx, uint_fast8_t flags)",
])

handlers = h_anim + "\n" + h_col + "\n" + h_amb

# Zaehlen, wie viele Pruefungen tatsaechlich drinstehen -- DIR-014: den Gegenstand nachzaehlen
found = handlers.count("esp8266_idx_ok")
sys.stderr.write("Pruefstand: %d esp8266_idx_ok-Aufrufe in den drei Handlern gefunden\n" % found)

if STRIP_GUARD:
    handlers, n = re.subn(
        r"    if \(! esp8266_idx_ok [^\n]*\n    \{\n        return;[^\n]*\n    \}\n\n", "", handlers)
    sys.stderr.write("Pruefstand: %d Pruefungen fuer den Fehlschlag-Nachweis entfernt\n" % n)
    if n == 0:
        sys.exit("Fehlschlag-Nachweis nicht herstellbar -- Muster trifft nicht")

HEAD = r'''
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include "display.h"
#include "log.h"

typedef struct { DISPLAY_GLOBALS d; unsigned char canary[16384]; } WRAP;
static WRAP wrap;
#define display (wrap.d)

static uint32_t esp8266_cmd_reject_cnt = 0;   /* derselbe Zaehler wie in main.c */
static unsigned long reject_lines = 0;
void log_printf (const char * fmt, ...) { (void) fmt; reject_lines++; }

static void display_save_animation (uint_fast8_t idx) { (void) idx; }
static void display_save_color_animation (uint_fast8_t idx) { (void) idx; }
static void display_save_ambilight_mode_deceleration (uint_fast8_t idx) { (void) idx; }

'''

TAIL = r'''
/* ----------------------------------------------------------------------------------------- */
static size_t off_anim, end_anim, off_col, end_col, off_amb, end_amb;
static unsigned char shadow[sizeof (WRAP)];
static size_t worst = 0;
static int violations = 0;

static void arm (void)
{
    memset (&wrap, 0xA5, sizeof (wrap));
    memcpy (shadow, &wrap, sizeof (wrap));
}

/* Meldet den hoechsten geschriebenen Offset und zaehlt jeden Schreibzugriff ausserhalb
   des erlaubten Fensters [lo, hi) als Verletzung. */
static void check (const char * what, unsigned idx, size_t lo, size_t hi)
{
    const unsigned char * now = (const unsigned char *) &wrap;
    size_t i;

    for (i = 0; i < sizeof (WRAP); i++)
    {
        if (now[i] != shadow[i])
        {
            if (i > worst) worst = i;

            if (i < lo || i >= hi)
            {
                violations++;
                if (violations <= 5)
                {
                    printf ("  VERLETZUNG %-9s idx=%3u schreibt auf Offset %zu "
                            "(erlaubt %zu..%zu, Struct endet bei %zu)\n",
                            what, idx, i, lo, hi - 1, sizeof (DISPLAY_GLOBALS) - 1);
                }
            }
        }
    }
}

int main (void)
{
    char line[16];
    unsigned idx;

    off_anim = offsetof (DISPLAY_GLOBALS, animations);
    end_anim = off_anim + sizeof (display.animations);
    off_col  = offsetof (DISPLAY_GLOBALS, color_animations);
    end_col  = off_col + sizeof (display.color_animations);
    off_amb  = offsetof (DISPLAY_GLOBALS, ambilight_modes);
    end_amb  = off_amb + sizeof (display.ambilight_modes);

    printf ("Grenzen aus den ECHTEN Headern: animations=%zu color_animations=%zu ambilight_modes=%zu\n",
            sizeof (display.animations) / sizeof (display.animations[0]),
            sizeof (display.color_animations) / sizeof (display.color_animations[0]),
            sizeof (display.ambilight_modes) / sizeof (display.ambilight_modes[0]));
    printf ("sizeof(DISPLAY_GLOBALS)=%zu, Kanarienvogel ab Offset %zu (zugesicherte Nachbarschaft im struct)\n",
            sizeof (DISPLAY_GLOBALS), offsetof (WRAP, canary));

    for (idx = 0; idx <= 255; idx++)
    {
        sprintf (line, "D%02X07", idx);  arm (); schedule_esp8266_animation_variable (line);       check ("AD", idx, off_anim, end_anim);
        sprintf (line, "F%02X03", idx);  arm (); schedule_esp8266_animation_variable (line);       check ("AF", idx, off_anim, end_anim);
        sprintf (line, "D%02X07", idx);  arm (); schedule_esp8266_color_animation_variable (line); check ("CD", idx, off_col,  end_col);
        sprintf (line, "D%02X07", idx);  arm (); schedule_esp8266_ambilight_mode (line);           check ("MD", idx, off_amb,  end_amb);
        sprintf (line, "F%02X03", idx);  arm (); schedule_esp8266_ambilight_mode (line);           check ("MF", idx, off_amb,  end_amb);
    }

    printf ("hoechster geschriebener Offset = %zu\n", worst);
    printf ("abgewiesene Zeilen gesamt (esp8266_cmd_reject_cnt) = %lu\n", (unsigned long) esp8266_cmd_reject_cnt);
    printf ("davon tatsaechlich auf die UART gemeldet (gedrosselt)  = %lu\n", reject_lines);
    printf ("VERLETZUNGEN = %d\n", violations);
    return violations ? 1 : 0;
}
'''

sys.stdout.write(
    HEAD + htoi + "\n" + setters + "\n" + idx_ok + "\n" + handlers + TAIL)
# Der erzeugte C-Text geht auf stdout -- der Aufrufer entscheidet, wohin damit.
# Ein fester Pfad im Scratchpad hat beim Uebernehmen genau einmal dazu gefuehrt,
# dass ein altes Binary eines anderen Laufs als eigenes Ergebnis gelesen wurde.
