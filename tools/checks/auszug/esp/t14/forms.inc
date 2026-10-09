/* TESTWERTE: jedes Hexfeld traegt nach Moeglichkeit einen Buchstaben -- sonst sind %x und %X gleich, und
 * eine geaenderte Formatangabe faellt nicht auf (bei der ersten Fassung so geschehen, DIR-014).
 * Jede aktive Kommandoform des ESP (V.1 (7), erhoben am Code 09.10.2026): 32 Stellen in vars.cpp
 * (die 33. -- 'm', set_num16_array -- steht in #if 0) und 1 in udpsrv.cpp (Tetris). Je Stelle ein
 * Aufruf, der GENAU diese Stelle ausloest. Wer eine Form hinzufuegt, ergaenzt hier -- run.sh zaehlt
 * die Stellen im Quelltext nach und vergleicht mit FORM_STELLEN. */
#define FORM_STELLEN 33
static char langname[200];
static void form_init () { memset (langname, 0, sizeof langname); for (int i = 0; i < 150; i++) langname[i] = 'a' + i % 26; }
static TM tmv = { 0, 30, 21, 4, 9, 2026 - 1900 + 1900, 0, 0, 0 };
static DSP_COLORS col = { 0xfe, 0x8d, 0xa1, 0x7c };
static void (* const formen[]) () = {
  [] { rpc ((RPC_VARIABLE) 2); },                                                   /* R  */
  [] { set_numvar ((NUM_VARIABLE) 0x0b, 0xbeaf); },                                    /* N  */
  [] { set_num8_array (DISPLAY_DIMMED_DISPLAY_COLORS, 0x0e, 0xab); },                  /* n  (1) */
  [] { set_num8_array (DISPLAY_DIMMED_AMBILIGHT_COLORS, 0x0a, 0xfc); },                /* n  (2) */
  [] { set_strvar ((STR_VARIABLE) 0, "Hallo*abcd Welt"); },                         /* S  Text auf '*' + Hex */
  [] { set_tm_var ((TM_VARIABLE) 0, &tmv); },                                       /* T  */
  [] { set_dsp_color_var ((DSP_COLOR_VARIABLE) 0, &col, 1); },                      /* DC rgbw */
  [] { set_dsp_color_var ((DSP_COLOR_VARIABLE) 0, &col, 0); },                      /* DC rgb */
  [] { set_display_animation_name (1, langname); },                                 /* AN, 150 Zeichen: Heap-Zweig */
  [] { set_display_animation_deceleration (1, 0xc9); },                                /* AD */
  [] { set_display_animation_default_deceleration (1, 0xd3); },                        /* AE */
  [] { set_display_animation_flags (1, 0xa1); },                                    /* AF */
  [] { set_color_animation_name ((COLOR_ANIMATION_VARIABLE) 1, (char *) "Regenbogen"); },   /* CN */
  [] { set_color_animation_deceleration ((COLOR_ANIMATION_VARIABLE) 1, 0xe4); },       /* CD */
  [] { set_color_animation_default_deceleration ((COLOR_ANIMATION_VARIABLE) 1, 0xf5); },  /* CE */
  [] { set_color_animation_flags ((COLOR_ANIMATION_VARIABLE) 1, 0xb2); },              /* CF */
  [] { set_ambilight_mode_name ((AMBILIGHT_MODE_VARIABLE) 1, (char *) "Lauflicht"); },  /* MN */
  [] { set_ambilight_mode_deceleration ((AMBILIGHT_MODE_VARIABLE) 1, 0xa6); },         /* MD */
  [] { set_ambilight_mode_default_deceleration ((AMBILIGHT_MODE_VARIABLE) 1, 0xb7); }, /* ME */
  [] { set_ambilight_mode_flags ((AMBILIGHT_MODE_VARIABLE) 1, 0xc1); },                /* MF */
  /* set_overlay_var: acht Zeilen OT OI OD OC OS OY ON OF aus EINEM Aufruf -- je eine Form */
  [] { strcpy (overlays[3].text, "Gute Nacht"); overlays[3].type = 0x0a; overlays[3].interval = 0xb5; overlays[3].duration = 0xc9;
       overlays[3].date_code = 0xd1; overlays[3].date_start = 0xac1f; overlays[3].days = 0x7f; overlays[3].flags = 0xe3; set_overlay_var (3); },
  [] { set_night_time_var (1, (NIGHT_TIME_VARIABLE) 2, 0x5ab, 0xa1); },              /* a  */
  [] { set_night_time_var (0, (NIGHT_TIME_VARIABLE) 2, 0x1ac, 0xb1); },               /* t  */
  [] { set_alarm_time_var ((ALARM_TIME_VARIABLE) 1, 0x1bc, 0xef); },                  /* l  */
  [] { set_ir_code_var (0x0c, 0xd2, 0xbeef, 0xa9f3); },                                   /* I  */
  [] { tetris_send ("gT3"); },                                                      /* G  (udpsrv.cpp) */
};
static const int formen_zeilen[] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 8, 1,1,1,1, 1 };
