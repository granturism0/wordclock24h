OVERLAY      overlays[MAX_OVERLAYS];

/*----------------------------------------------------------------------------------------------------------------------------------------
 * var_cmd_min_len () - Mindestlaenge einer Kommandozeile, Befund L237
 *
 * Hintergrund: In var_set_parameter() folgt auf jedes htoi (parameters, 2) ein UNBEDINGTES
 * parameters += 2 -- an rund 45 Stellen nach demselben Bauplan. Stand der Zeiger bereits auf
 * dem Terminator, zeigt er danach DAHINTER, und der naechste htoi liest dort.
 *
 * Der Fix aus L232 begrenzt den Schaden -- htoi haelt am ersten Nullbyte --, hebt ihn aber
 * nicht auf: Hinter dem Terminator steht nicht zwingend eine weitere Null. Sie steht dort
 * sogar meistens NICHT. Die Zeile liegt in cmd_buffer, einem static char [CMD_BUFFER_SIZE]
 * im Hauptloop (ESP-uclock.ino:499). Der wird nie geleert, nur an cmd_len terminiert --
 * dahinter liegt der Rest des VORIGEN Kommandos. Eine um ein Zeichen verkuerzte Zeile liest
 * also Text aus dem Kommando davor und macht daraus einen gueltig aussehenden falschen Wert.
 *
 * Nachgerechnet und am Pruefstand nachgestellt: In cmd_buffer steht noch "var ON00Weihnachten"
 * (Index 8 = 'W', 9 = 'e'). Danach trifft statt "var OT0002" nur "var OT0" ein -- drei Zeichen
 * auf der Bruecke verloren. Der Terminator liegt jetzt auf Index 7, die Buchstaben ab Index 8
 * stehen unveraendert aus dem vorigen Kommando da. Ablauf: cmd_code = 'O' (Index 4),
 * cmd_code = 'T' (Index 5), var_idx = htoi (Index 6, 2) = 0 -- haelt am Terminator, also noch
 * harmlos --, dann parameters += 2 UNBEDINGT, und der Zeiger steht auf Index 8, also HINTER
 * dem Terminator. type = htoi ("We", 2): 'W' ist keine Hexziffer und zaehlt als 0, 'e' ist 14.
 *
 *     type = 14.
 *
 * Das ist nicht ungefaehr der Fall aus L205, das IST der Wert aus L205: Dort kam
 * overlay[0].type als 14 statt 2 an und sah gueltig genug aus, um durch jede Pruefung zu
 * kommen. Gueltig ist er nicht -- src/overlay/overlay.h laesst 0..10 zu.
 *
 * Gewaehlte Loesung: EINE Laengenpruefung vorne statt 45 Einzelpruefungen. Jede Kommandoart
 * hat eine feste Mindestbreite; passt sie nicht, wird das Kommando GANZ verworfen statt halb
 * ausgefuehrt. Halb ausgefuehrt ist hier die schlechtere Haelfte: Ein fehlendes Kommando
 * laesst den alten Wert stehen und faellt beim naechsten Abgleich auf, ein halb ausgefuehrtes
 * schreibt einen falschen und faellt nirgends auf.
 *
 * Warum nicht 45 Einzelpruefungen: Das waeren 45 Gelegenheiten, eine zu vergessen, und die
 * naechste hinzugefuegte Kommandoart haette wieder keine. Diese Tabelle steht unmittelbar vor
 * dem switch, den sie beschreibt -- weicht eine Breite ab, faellt es beim Lesen auf.
 *
 * Rueckgabe: Mindestzahl Zeichen ab parameters[0] EINSCHLIESSLICH Kommandobuchstabe.
 *            0 = Kommandobuchstabe unbekannt. Dann wird nichts geprueft, weil der switch in
 *            var_set_parameter() die Zeile ohnehin wirkungslos verwirft.
 *
 * parameters[0] ist beim Aufruf garantiert != '\0' (der Aufrufer prueft es), parameters[1]
 * darf deshalb gelesen werden -- schlimmstenfalls ist es der Terminator, und der trifft
 * keinen case.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
var_cmd_min_len (const char * parameters)
{
    switch (parameters[0])
    {
        case CMD_CODE_NUMERIC_VAR:                  return  7;              // N ii ll hh
        case CMD_CODE_NUMERIC_ARRAY:                return  7;              // n ii nn bb
        case CMD_CODE_STRING_VAR:                   return  3;              // S ii + Text, Text darf leer sein
        case CMD_CODE_TIME_VAR:                     return 17;              // T ii YYYYMMDDhhmmss, Ziffern direkt indiziert
        case CMD_CODE_NIGHT_TIME_TABLE:             return  9;              // t ii mmmm ff
        case CMD_CODE_AMBILIGHT_NIGHT_TIME_TABLE:   return  9;              // a ii mmmm ff
        case CMD_CODE_ALARM_TIME_TABLE:             return  9;              // l ii mmmm ff
        case CMD_CODE_IR_CODE:                      return 13;              // I ii pp aaaa cccc

        case CMD_CODE_DISPLAY_VAR:                                          // D
            if (parameters[1] == PAR_CODE_DISPLAY_COLOR)                    // DC ii rr gg bb ww
            {
                /* Immer 12, NICHT von DISPLAY_USE_RGBW_NUM_VAR abhaengig. Der switch unten
                 * liest das ww zwar nur bei RGBW, der STM sendet es aber in beiden Bauarten:
                 * "DC%02x%02x%02x%02x%02x" mit White, sonst "DC%02x%02x%02x%02x00" mit einer
                 * literalen 00 (src/vars/vars.c:426/428, seit dem ersten Checkin unveraendert).
                 *
                 * Ein erster Entwurf machte die Breite hier von get_numvar() abhaengig. Das
                 * waere eine Pruefung gewesen, die von einem Wert abhaengt, den dieselbe
                 * Bruecke liefert -- geht das use_rgbw-Kommando verloren, verwirft der ESP
                 * danach gueltige Farbkommandos. Eine Konstante kann das nicht.
                 */
                return 12;
            }
            return 4;                                                       // unbekannter Unterbuchstabe: nur der Kopf

        case CMD_CODE_ANIMATION_VAR:                                        // A
        case CMD_CODE_COLOR_ANIMATION_VAR:                                  // C
        case CMD_CODE_AMBILIGHT_MODE_VAR:                                   // M
            switch (parameters[1])                                          // N/D/E/F -- in allen drei Gruppen dieselben
            {                                                               // Buchstaben, siehe die PAR_CODE_*-Defines oben
                case PAR_CODE_ANIMATION_MODE_NAME:              return 4;   // xN ii + Name, Name darf leer sein
                case PAR_CODE_ANIMATION_DECELERATION:           return 6;   // xD ii <wert:2>
                case PAR_CODE_ANIMATION_DEFAULT_DECELERATION:   return 6;   // xE ii <wert:2>
                case PAR_CODE_ANIMATION_FLAGS:                  return 6;   // xF ii <wert:2>
            }
            return 4;

        case CMD_CODE_OVERLAY_VAR:                                          // O
            switch (parameters[1])
            {
                case PAR_CODE_OVERLAY_TEXT:                     return 4;   // ON ii + Text, Text darf leer sein
                case PAR_CODE_OVERLAY_DATE_START:               return 8;   // OS ii <wert:4> -- als einziges vier Stellen
                case PAR_CODE_OVERLAY_TYPE:                     return 6;
                case PAR_CODE_OVERLAY_INTERVAL:                 return 6;
                case PAR_CODE_OVERLAY_DURATION:                 return 6;
                case PAR_CODE_OVERLAY_DATE_CODE:                return 6;
                case PAR_CODE_OVERLAY_DAYS:                     return 6;
                case PAR_CODE_OVERLAY_FLAGS:                    return 6;
            }
            return 4;
    }

    return 0;                                                               // unbekanntes Kommando
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * var_cmd_reject () - ein verworfenes Kommando festhalten, Befund L237
 *
 * Es darf nicht still verschwinden. Es darf aber auch nicht die Bruecke zusaetzlich belasten:
 * L109 beschreibt genau diese Mitkopplung -- eine Meldung ueber einen Uebertragungsfehler legt
 * Last auf dieselbe Leitung, deren Ueberlastung den Fehler erzeugt hat, und der RX-Ring des
 * STM ist 256 Byte gross und verwirft bei Ueberlauf still.
 *
 * Deshalb AUSSCHLIESSLICH stm32_log_append(): ein RAM-Ring im ESP, abrufbar ueber
 * /api/stm32_log, Kosten auf der UART null. Kein Serial.println, kein debugmsg.
 *
 * Gedrosselt, weil der Ring nur STM32_LOG_LINES Zeilen fasst und auch die Diagnosezeilen
 * traegt: die ersten vier Faelle einzeln, danach jeder fuenfzigste. Der laufende Zaehler steht
 * IN der Zeile -- die Gesamtzahl geht also nicht verloren, auch wenn nur die letzte uebrig ist.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static uint32_t     var_cmd_reject_cnt = 0;

static void
var_cmd_reject (const char * parameters, unsigned int have, unsigned int want)
{
    var_cmd_reject_cnt++;

    if (var_cmd_reject_cnt <= 4 || (var_cmd_reject_cnt % 50) == 0)
    {
        char line[80];

        snprintf (line, sizeof (line), "- var verworfen #%lu len=%u<%u: %.32s",
                  (unsigned long) var_cmd_reject_cnt, have, want, parameters);
        stm32_log_append (line);
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * var_ignored_note () - eine still entfallene Zuweisung zaehlen und gedrosselt melden, A35 (b)
 *
 * specs/paket-2026-10-11, design.md 9, Ent-5 = b. var_set_parameter() quittiert eine formal verwertbare Zeile
 * mit dem Punkt, auch wenn sie NICHTS zuweist: Index >= MAX, Wert ausser Bereich (n: > MAX_BRIGHTNESS, Element
 * ausser Bereich), unbekannter Unterbuchstabe (D/A/C/M/O) oder unbekannter Kommandobuchstabe. Das bleibt so --
 * Rueckgabe unveraendert, kein NAK: Eine Wiederholung aenderte daran nichts (L233). Aber es geschieht nicht mehr
 * spurlos.
 *
 * Gezaehlt wird JEDE solche Zeile (var_ignored_cnt, saettigend). Gemeldet wird die erste sofort, danach hoechstens
 * eine je VAR_IGNORED_LOG_INTERVAL; der laufende Zaehler steht in der Zeile, die Gesamtzahl geht also nicht verloren.
 *
 *     "- var ignoriert #<n> code=<X|XY> idx=<hh|-->"
 *
 * code: Kommandobuchstabe, bei D/A/C/M/O samt Unterbuchstabe; nicht druckbare Zeichen als '?'. idx: der Index
 * der Zeile in zwei Hexziffern, "--" beim unbekannten Kommandobuchstaben (dort ist nichts ausgewertet). OHNE Wert
 * (A20): Die S-Zeilen tragen Zugangsdaten.
 *
 * NICHT gezaehlt: "var V..." -- die Rahmenzeilen, die ein alter ESP regulaer quittieren und verwerfen muss
 * (ESP-uclock.ino, Kopfkommentar der Pruefsumme, Punkt 1). Ein neuer ESP faengt sie vorher in var_frame_line()
 * ab; die Ausnahme hier gilt trotzdem, damit eine kuenftige Aenderung daran keine Meldung im Normalbetrieb erzeugt.
 * Ebenfalls NICHT: die zu kurze Zeile -- die ist nicht still, sondern "!v" samt var_cmd_reject().
 *
 * WEG DER MELDUNG: Mitschnitt UND /api/stm32_log, wie die Heap-Zeile (ESP-uclock.ino, esp_heap_log()).
 * stm32_log_append() allein erreicht den Mitschnitt nicht (L185); Serial.println mit "- " erreicht ihn ueber den
 * STM (ESP8266_DEBUGMSG). Die STM-Warteschleife in var_send_buf() wartet bei einer "- "-Zeile weiter, die Zeile
 * vor der Quittung stoert die Paarung nicht. Kosten: hoechstens eine Zeile (<= 46 Byte) je Minute.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#define VAR_IGNORED_LOG_INTERVAL    60000UL                                 // msec

static uint32_t         var_ignored_cnt         = 0;
static uint32_t    var_ignored_last        = 0;
static uint_fast8_t     var_ignored_logged      = 0;

static char
var_ignored_char (char ch)
{
    return (ch > ' ' && ch <= '~') ? ch : '?';
}

static void
var_ignored_note (const char * cmd_line, uint_fast8_t known, uint_fast8_t var_idx)
{
    if (var_ignored_cnt < 0xFFFFFFFFUL)                                     // saettigend
    {
        var_ignored_cnt++;
    }

    if (! var_ignored_logged || (millis () - var_ignored_last) >= VAR_IGNORED_LOG_INTERVAL)
    {
        char    code[3];
        char    idx[3];
        char    line[56];                                                   // "- var ignoriert #4294967295 code=XY idx=--" = 42

        var_ignored_logged  = 1;
        var_ignored_last    = millis ();

        code[0] = var_ignored_char (cmd_line[0]);
        code[1] = '\0';
        code[2] = '\0';

        if (known && strchr ("DACMO", cmd_line[0]))                         // zweibuchstabige Kommandos
        {
            code[1] = var_ignored_char (cmd_line[1]);
        }

        if (known)
        {
            snprintf_P (idx, sizeof (idx), PSTR ("%02x"), (unsigned int) (var_idx & 0xFF));
        }
        else
        {
            strcpy (idx, "--");
        }

        snprintf_P (line, sizeof (line), PSTR ("- var ignoriert #%lu code=%s idx=%s"), (unsigned long) var_ignored_cnt, code, idx);
        Serial.println (line);
        Serial.flush ();
        stm32_log_append (line);
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * var_set_parameter () - eine Kommandozeile der Bruecke auswerten
 *
 * Rueckgabe: 1 = Zeile war formal verwertbar, 0 = verworfen, weil zu kurz fuer ihre Kommandoart.
 *
 * Das heisst bewusst NICHT "Wert uebernommen": Eine formal gueltige Zeile mit unbekanntem
 * Kommandobuchstaben liefert ebenfalls 1, obwohl sie nichts bewirkt -- eine Wiederholung
 * wuerde daran nichts aendern. Dass die Quittung in ESP-uclock.ino nur den Empfang bestaetigt
 * und nicht die Uebernahme, bleibt der offene Punkt L233 und wird hier nicht geloest.
 *
 * Der Rueckgabewert ist der Haken aus A32 (specs/bruecke-wiederholung), und er WIRD seit
 * A32 ausgewertet: Der var-Zweig in ESP-uclock.ino antwortet bei 0 mit "!v" statt mit ".",
 * damit der STM die Zeile nachsendet -- das ist der Mechanismus, mit dem L205 geschlossen
 * wurde. Hier stand bis zum 05.10.2026 das Gegenteil ("Heute wertet ihn niemand aus"), und
 * das war keine verschobene Zeilennummer, sondern eine falsche Zusicherung im Quelltext
 * (L274): Wer nur den Kommentar liest, haelt den Rueckkanal fuer ungebaut.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
var_set_parameter (char * parameters)
{
    uint_fast8_t        cmd_code;
    uint_fast8_t        var_idx = 0;
    uint_fast8_t        applied = 0;                                        // A35: 1 = mindestens eine Zuweisung erfolgt
    const char *        cmd_line;                                           // A35: Zeilenanfang fuer die Meldung
    uint_fast8_t        min_len;
    size_t              len;

#ifdef DEBUG
    debugmsg ("VAR", parameters);
#endif

    /* L237: EINE Pruefung vorne statt 45 einzelner, siehe var_cmd_min_len() darueber.
     * Ab hier gilt: Die Zeile ist lang genug fuer ihre Kommandoart. Jedes parameters += 2
     * im folgenden switch bleibt damit innerhalb der Zeile, und kein htoi liest hinter dem
     * Terminator.
     */
    len     = strlen (parameters);
    min_len = len ? var_cmd_min_len (parameters) : 1;                       // leere Zeile: immer verwerfen

    if (len < min_len)
    {
        var_cmd_reject (parameters, (unsigned int) len, (unsigned int) min_len);
        return 0;
    }

    cmd_line    = parameters;
    cmd_code    = *parameters++;

    switch (cmd_code)
    {
        case CMD_CODE_NUMERIC_VAR:                                          // N: numeric variable: Niillhh
        {
            uint_fast8_t    lo;
            uint_fast8_t    hi;
            uint_fast16_t   val;

            var_idx = htoi (parameters, 2);
            parameters += 2;
            lo      = htoi (parameters, 2);
            parameters += 2;
            hi      = htoi (parameters, 2);
            parameters += 2;
            val     = (hi << 8) | lo;

            if (var_idx < MAX_NUM_VARIABLES)
            {
                numvars[var_idx] = val;
                applied = 1;
            }
            break;
        }

        case CMD_CODE_NUMERIC_ARRAY:                                        // A: numeric array: Niinnbb
        {
            uint_fast8_t    n;
            uint_fast8_t    val;

            var_idx = htoi (parameters, 2);
            parameters += 2;

            n = htoi (parameters, 2);
            parameters += 2;

            val = htoi (parameters, 2);

            switch (var_idx)
            {
                case DISPLAY_DIMMED_DISPLAY_COLORS:
                {
                    if (n < sizeof (dimmed_display_colors))
                    {
                        if (val <= MAX_BRIGHTNESS)
                        {
                            dimmed_display_colors[n] = val;
                            applied = 1;
                        }
                    }
                    break;
                }
                case DISPLAY_DIMMED_AMBILIGHT_COLORS:
                {
                    if (n < sizeof (dimmed_ambilight_colors))
                    {
                        if (val <= MAX_BRIGHTNESS)
                        {
                            dimmed_ambilight_colors[n] = val;
                            applied = 1;
                        }
                    }
                    break;
                }
            }
            break;
        }

        case CMD_CODE_STRING_VAR:                                           // S string variable: Siissssssss...
        {
            var_idx = htoi (parameters, 2);
            parameters += 2;

            if (var_idx < MAX_STR_VARIABLES)
            {
                memset (strvars[var_idx].str, 0, strvars[var_idx].maxlen + 1);
                strncpy (strvars[var_idx].str, parameters, strvars[var_idx].maxlen);
                strvars[var_idx].str[strvars[var_idx].maxlen] = '\0';
                applied = 1;
            }
            break;
        }

        case CMD_CODE_TIME_VAR:                                             // T time: TYYYYMMDDhhmmss
        {
            var_idx = htoi (parameters, 2);
            parameters += 2;

            if (var_idx < MAX_TM_VARIABLES)
            {
                tmvars[var_idx].tm_year = 1000 * (parameters[0]  - '0') +
                                           100 * (parameters[1]  - '0') +
                                            10 * (parameters[2]  - '0') +
                                             1 * (parameters[3]  - '0');
                tmvars[var_idx].tm_mon  =   10 * (parameters[4]  - '0') +
                                             1 * (parameters[5]  - '0');
                tmvars[var_idx].tm_mday =   10 * (parameters[6]  - '0') +
                                             1 * (parameters[7]  - '0');
                tmvars[var_idx].tm_hour =   10 * (parameters[8]  - '0') +
                                             1 * (parameters[9]  - '0');
                tmvars[var_idx].tm_min  =   10 * (parameters[10] - '0') +
                                             1 * (parameters[11] - '0');
                tmvars[var_idx].tm_sec =    10 * (parameters[12] - '0') +
                                             1 * (parameters[13] - '0');

                tmvars[var_idx].tm_wday = dayofweek (tmvars[var_idx].tm_mday, tmvars[var_idx].tm_mon + 1, tmvars[var_idx].tm_year + 1900);
                applied = 1;
            }

            break;
        }

        case CMD_CODE_DISPLAY_VAR:                                          // D: display
        {
            cmd_code = *parameters++;
            var_idx = htoi (parameters, 2);
            parameters += 2;

            switch (cmd_code)
            {
                case PAR_CODE_DISPLAY_COLOR:                                // DC: Display Color
                {
                    uint_fast8_t use_rgbw = get_numvar (DISPLAY_USE_RGBW_NUM_VAR);

                    if (var_idx < MAX_DSP_COLOR_VARIABLES)
                    {
                        dspcolorvars[var_idx].red = htoi (parameters, 2);
                        parameters += 2;
                        dspcolorvars[var_idx].green = htoi (parameters, 2);
                        parameters += 2;
                        dspcolorvars[var_idx].blue = htoi (parameters, 2);
                        applied = 1;
                        parameters += 2;

                        if (use_rgbw)
                        {
                            dspcolorvars[var_idx].white = htoi (parameters, 2);
                            parameters += 2;
                        }
                    }
                    break;
                }
            }
            break;
        }

        case CMD_CODE_ANIMATION_VAR:                                        // A: animation
        {
            cmd_code = *parameters++;
            var_idx = htoi (parameters, 2);
            parameters += 2;

            if (var_idx < MAX_DISPLAY_ANIMATION_VARIABLES)
            {
                if (max_display_animation_variables < var_idx + 1)
                {
                    max_display_animation_variables = var_idx + 1;
                }
  
                switch (cmd_code)
                {
                    case PAR_CODE_ANIMATION_MODE_NAME:                          // AN: Animation mode Name
                    {
                        strncpy (displayanimationvars[var_idx].name, parameters, MAX_DISPLAY_ANIMATION_NAME_LEN);
                        applied = 1;
                        break;
                    }
    
                    case PAR_CODE_ANIMATION_DECELERATION:                       // AD: Animation Deceleration
                    {
                        uint_fast8_t deceleration = htoi (parameters, 2);
                        parameters += 2;
                        displayanimationvars[var_idx].deceleration = deceleration;
                        applied = 1;
                        break;
                    }
    
                    case PAR_CODE_ANIMATION_DEFAULT_DECELERATION:               // AE: Animation default deceleration
                    {
                        uint_fast8_t default_deceleration = htoi (parameters, 2);
                        parameters += 2;
                        displayanimationvars[var_idx].default_deceleration = default_deceleration;
                        applied = 1;
                        break;
                    }
    
                    case PAR_CODE_ANIMATION_FLAGS:                              // AF: Animation Flags
                    {
                        uint_fast8_t flags = htoi (parameters, 2);
                        parameters += 2;
                        displayanimationvars[var_idx].flags = flags;
                        applied = 1;
                        break;
                    }
                }
            }
            break;
        }

        case CMD_CODE_COLOR_ANIMATION_VAR:                                  // C: Color animation
        {
            cmd_code = *parameters++;
            var_idx = htoi (parameters, 2);
            parameters += 2;

            switch (cmd_code)
            {
                case PAR_CODE_COLOR_ANIMATION_MODE_NAME:                    // CN: Color animation Name
                {
                    if (var_idx < MAX_COLOR_ANIMATION_VARIABLES)
                    {
                        strncpy (coloranimationvars[var_idx].name, parameters, MAX_COLOR_ANIMATION_NAME_LEN);
                        applied = 1;
                    }
                    break;
                }

                case PAR_CODE_COLOR_ANIMATION_DECELERATION:                 // CD: Color animation Deceleration
                {
                    if (var_idx < MAX_COLOR_ANIMATION_VARIABLES)
                    {
                        uint_fast8_t deceleration = htoi (parameters, 2);
                        parameters += 2;
                        coloranimationvars[var_idx].deceleration = deceleration;
                        applied = 1;
                    }
                    break;
                }

                case PAR_CODE_COLOR_ANIMATION_DEFAULT_DECELERATION:         // CE: Color animation default deceleration
                {
                    if (var_idx < MAX_COLOR_ANIMATION_VARIABLES)
                    {
                        uint_fast8_t default_deceleration = htoi (parameters, 2);
                        parameters += 2;
                        coloranimationvars[var_idx].default_deceleration = default_deceleration;
                        applied = 1;
                    }
                    break;
                }

                case PAR_CODE_COLOR_ANIMATION_FLAGS:                        // CF: Animation Flags
                {
                    if (var_idx < MAX_COLOR_ANIMATION_VARIABLES)
                    {
                        uint_fast8_t flags = htoi (parameters, 2);
                        parameters += 2;
                        coloranimationvars[var_idx].flags = flags;
                        applied = 1;
                    }
                    break;
                }
            }
            break;
        }

        case CMD_CODE_AMBILIGHT_MODE_VAR:                                   // Ambilight mode
        {
            cmd_code = *parameters++;
            var_idx = htoi (parameters, 2);
            parameters += 2;

            switch (cmd_code)
            {
                case PAR_CODE_AMBILIGHT_MODE_NAME:                          // MN: Ambilight mode Name
                {
                    if (var_idx < MAX_AMBILIGHT_MODE_VARIABLES)
                    {
                        strncpy (ambilightmodevars[var_idx].name, parameters, MAX_AMBILIGHT_MODE_NAME_LEN);
                        applied = 1;
                    }
                    break;
                }

                case PAR_CODE_AMBILIGHT_MODE_DECELERATION:                  // MD: Ambilight mode Deceleration
                {
                    if (var_idx < MAX_AMBILIGHT_MODE_VARIABLES)
                    {
                        uint_fast8_t deceleration = htoi (parameters, 2);
                        parameters += 2;
                        ambilightmodevars[var_idx].deceleration = deceleration;
                        applied = 1;
                    }
                    break;
                }

                case PAR_CODE_AMBILIGHT_MODE_DEFAULT_DECELERATION:          // ME: Ambilight mode default deceleration
                {
                    if (var_idx < MAX_AMBILIGHT_MODE_VARIABLES)
                    {
                        uint_fast8_t default_deceleration = htoi (parameters, 2);
                        parameters += 2;
                        ambilightmodevars[var_idx].default_deceleration = default_deceleration;
                        applied = 1;
                    }
                    break;
                }

                case PAR_CODE_AMBILIGHT_MODE_FLAGS:                         // MF: Ambilight mode Flags
                {
                    if (var_idx < MAX_AMBILIGHT_MODE_VARIABLES)
                    {
                        uint_fast8_t flags = htoi (parameters, 2);
                        parameters += 2;
                        ambilightmodevars[var_idx].flags = flags;
                        applied = 1;
                    }
                    break;
                }
            }
            break;
        }

        case CMD_CODE_OVERLAY_VAR:                                          // O: overlay
        {
            cmd_code = *parameters++;
            var_idx = htoi (parameters, 2);
            parameters += 2;

            switch (cmd_code)
            {
                case PAR_CODE_OVERLAY_TYPE:                                 // OT: overlay type
                    if (var_idx < MAX_OVERLAYS)
                    {
                        uint_fast8_t type = htoi (parameters, 2);
                        parameters += 2;
                        overlays[var_idx].type = type;
                        applied = 1;
                    }
                    break;
                case PAR_CODE_OVERLAY_INTERVAL:                             // OI: overlay interval
                    if (var_idx < MAX_OVERLAYS)
                    {
                        uint_fast8_t interval = htoi (parameters, 2);
                        parameters += 2;
                        overlays[var_idx].interval = interval;
                        applied = 1;
                    }
                    break;
                case PAR_CODE_OVERLAY_DURATION:                             // OD: overlay duration
                    if (var_idx < MAX_OVERLAYS)
                    {
                        uint_fast8_t duration = htoi (parameters, 2);
                        parameters += 2;
                        overlays[var_idx].duration = duration;
                        applied = 1;
                    }
                    break;
                case PAR_CODE_OVERLAY_DATE_CODE:                            // OC: overlay date code
                    if (var_idx < MAX_OVERLAYS)
                    {
                        uint_fast8_t date_code = htoi (parameters, 2);
                        parameters += 2;
                        overlays[var_idx].date_code = date_code;
                        applied = 1;
                    }
                    break;
                case PAR_CODE_OVERLAY_DATE_START:                           // OS: overlay date start
                    if (var_idx < MAX_OVERLAYS)
                    {
                        uint_fast16_t date_start = htoi (parameters, 4);
                        parameters += 4;
                        overlays[var_idx].date_start = date_start;
                        applied = 1;
                    }
                    break;
                case PAR_CODE_OVERLAY_DAYS:                                 // OY: overlay date end
                    if (var_idx < MAX_OVERLAYS)
                    {
                        uint_fast16_t days = htoi (parameters, 2);
                        parameters += 2;
                        overlays[var_idx].days = days;
                        applied = 1;
                    }
                    break;
                case PAR_CODE_OVERLAY_TEXT:                                 // ON: overlay name or text
                    if (var_idx < MAX_OVERLAYS)
                    {
                        strncpy (overlays[var_idx].text, parameters, OVERLAY_MAX_TEXT_LEN);
                        applied = 1;
                    }
                    break;
                case PAR_CODE_OVERLAY_FLAGS:                                // OF: overlay flags
                    if (var_idx < MAX_OVERLAYS)
                    {
                        uint_fast8_t flags = htoi (parameters, 2);
                        parameters += 2;
                        overlays[var_idx].flags = flags;
                        applied = 1;
                    }
                    break;
            }
            break;
        }

        case CMD_CODE_NIGHT_TIME_TABLE:                                     // tiimm night table: minutes + flags
        case CMD_CODE_AMBILIGHT_NIGHT_TIME_TABLE:                           // aiimm ambilight night table: minutes + flags
        {
            uint_fast16_t minutes;
            uint_fast8_t flags;

            var_idx = htoi (parameters, 2);
            parameters += 2;
            minutes = htoi (parameters, 2) + (htoi (parameters + 2, 2) << 8);
            parameters += 4;
            flags = htoi (parameters, 2);
            parameters += 2;

            if (var_idx < MAX_NIGHT_TIME_VARIABLES)
            {
                applied = 1;
                if (cmd_code == 't')
                {
                    nighttimevars[var_idx].minutes = minutes;
                    nighttimevars[var_idx].flags = flags;
                }
                else
                {
                    ambilightnighttimevars[var_idx].minutes = minutes;
                    ambilightnighttimevars[var_idx].flags = flags;
                }
            }

            break;
        }

        case CMD_CODE_ALARM_TIME_TABLE:                                     // liimm alarm table: minutes + flags
        {
            uint_fast16_t minutes;
            uint_fast8_t flags;

            var_idx = htoi (parameters, 2);
            parameters += 2;
            minutes = htoi (parameters, 2) + (htoi (parameters + 2, 2) << 8);
            parameters += 4;
            flags = htoi (parameters, 2);
            parameters += 2;

            if (var_idx < MAX_ALARM_TIME_VARIABLES)
            {
                alarmtimevars[var_idx].minutes = minutes;
                alarmtimevars[var_idx].flags = flags;
                applied = 1;
            }

            break;
        }

        case CMD_CODE_IR_CODE:                                              // I<idx:2><protocol:2><address:4><command:4>
        {
            uint_fast8_t    protocol;
            uint_fast16_t   address;
            uint_fast16_t   command;

            var_idx = htoi (parameters, 2);                                 // feste Breiten, siehe var_send_ir_code() im STM
            parameters += 2;
            protocol = htoi (parameters, 2);
            parameters += 2;
            address = htoi (parameters, 4);
            parameters += 4;
            command = htoi (parameters, 4);
            parameters += 4;

            if (var_idx < MAX_IR_CODES)
            {
                ir_codes[var_idx].protocol  = protocol;
                ir_codes[var_idx].address   = address;
                ir_codes[var_idx].command   = command;
                ir_codes_received_mask     |= (((uint32_t) 1) << var_idx);
                applied = 1;
            }
            /* Ein Index jenseits von MAX_IR_CODES wird verworfen -- aber nicht spurlos: Sein Bit
             * fehlt in der Maske, ir_codes_is_complete() bleibt 0 und der Endpunkt nennt ihn in
             * missing[]. Der Abzug scheitert damit sichtbar statt halb zu gelingen.
             */

            break;
        }
    }

    /* A35 (b): nichts zugewiesen -- zaehlen und gedrosselt melden. Die Rueckgabe bleibt 1 (L233). "var V..." zaehlt
     * nicht, siehe var_ignored_note(). known: var_cmd_min_len() kennt den Kommandobuchstaben, dann ist var_idx
     * in jedem Zweig ausgewertet.
     */
    if (! applied && cmd_line[0] != 'V')
    {
        var_ignored_note (cmd_line, min_len ? 1 : 0, var_idx);
    }

    return 1;
}

