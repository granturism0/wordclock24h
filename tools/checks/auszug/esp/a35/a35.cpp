/* Pruefstand a35 (Paket 2026-10-11, E.1, A35 (b), Ent-5): var_set_parameter() zaehlt still entfallene
 * Zuweisungen und meldet sie gedrosselt als "- var ignoriert #n code=X idx=yy" -- im Mitschnitt (Serial)
 * UND im API-Ring (stm32_log_append). "V..." zaehlt nicht, die Rueckgabe bleibt unveraendert, kein Wert
 * in der Zeile (A20).
 *
 * Die Erwartung ist UNABHAENGIG formuliert: Jede Zeile traegt "wird zugewiesen" (ja/nein) aus der Tabelle
 * unten; der Pruefstand stellt ausserdem nach, OB die Zuweisung im Speicher angekommen ist, und prueft
 * beides gegeneinander (eine "ja"-Zeile muss den Wert setzen, eine "nein"-Zeile darf nichts setzen).
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "pruefstand.h"
#define uint_fast8_t  unsigned int
#define uint_fast16_t unsigned int
#include VARSHDR
/* Flash-Ablage (Review E.6): PSTR/snprintf_P legen auf dem ESP nur das Format in den Flash; der Text ist byte-gleich. */
#define PSTR(s)     (s)
#define snprintf_P  snprintf

static zp_millis fake_ms = 1000;
static zp_millis millis (void) { return fake_ms; }
struct SerialStub {
    std::string out;
    void println (const char * s) { out += s; out += "\r\n"; }
    void print (const char * s) { out += s; }
    void flush (void) {}
} Serial;
static std::vector<std::string> logring;
void stm32_log_append (const char * l) { logring.push_back (l); }

unsigned int    numvars[MAX_NUM_VARIABLES];
unsigned int    get_numvar (NUM_VARIABLE v) { return v < MAX_NUM_VARIABLES ? numvars[v] : 0; }
uint8_t         max_display_animation_variables = 0;
uint8_t         dimmed_display_colors[MAX_BRIGHTNESS + 1];
uint8_t         dimmed_ambilight_colors[MAX_BRIGHTNESS + 1];
static char     sbuf[MAX_STR_VARIABLES][80];
STR_VAR         strvars[MAX_STR_VARIABLES];
TM              tmvars[MAX_TM_VARIABLES];
DSP_COLORS      dspcolorvars[MAX_DSP_COLOR_VARIABLES];
DISPLAY_ANIMATION displayanimationvars[MAX_DISPLAY_ANIMATION_VARIABLES];
COLOR_ANIMATION coloranimationvars[MAX_COLOR_ANIMATION_VARIABLES];
AMBILIGHT_MODE  ambilightmodevars[MAX_AMBILIGHT_MODE_VARIABLES];
NIGHT_TIME      nighttimevars[MAX_NIGHT_TIME_VARIABLES];
NIGHT_TIME      ambilightnighttimevars[MAX_NIGHT_TIME_VARIABLES];
ALARM_TIME      alarmtimevars[MAX_ALARM_TIME_VARIABLES];
static IR_CODE  ir_codes[MAX_IR_CODES];
static uint32_t ir_codes_received_mask;

#include "base.inc"
#include "defs.inc"
#include "code.inc"

/* Schnappschuss aller Ziele: Ist eine Zuweisung angekommen? */
struct Snap { std::vector<unsigned char> b; };
#define ADD(x) do { const unsigned char * p = (const unsigned char *) &(x); s.b.insert (s.b.end (), p, p + sizeof (x)); } while (0)
static Snap snap (void)
{
    Snap s;
    ADD (numvars); ADD (dimmed_display_colors); ADD (dimmed_ambilight_colors); ADD (sbuf); ADD (tmvars);
    ADD (dspcolorvars); ADD (displayanimationvars); ADD (coloranimationvars); ADD (ambilightmodevars);
    ADD (nighttimevars); ADD (ambilightnighttimevars); ADD (alarmtimevars); ADD (ir_codes); ADD (ir_codes_received_mask);
    ADD (overlays);
    return s;
}
/* Vorbelegung mit einem Muster, das kein Testwert trifft -- sonst saehe eine Zuweisung gleichen Werts wie "nichts" aus */
static void fill (void)
{
    memset (numvars, 0xA5, sizeof numvars); memset (dimmed_display_colors, 0xA5, sizeof dimmed_display_colors);
    memset (dimmed_ambilight_colors, 0xA5, sizeof dimmed_ambilight_colors); memset (sbuf, 0x5A, sizeof sbuf);
    memset (tmvars, 0xA5, sizeof tmvars); memset (dspcolorvars, 0xA5, sizeof dspcolorvars);
    memset (displayanimationvars, 0xA5, sizeof displayanimationvars); memset (coloranimationvars, 0xA5, sizeof coloranimationvars);
    memset (ambilightmodevars, 0xA5, sizeof ambilightmodevars); memset (nighttimevars, 0xA5, sizeof nighttimevars);
    memset (ambilightnighttimevars, 0xA5, sizeof ambilightnighttimevars); memset (alarmtimevars, 0xA5, sizeof alarmtimevars);
    memset (ir_codes, 0xA5, sizeof ir_codes); ir_codes_received_mask = 0; memset (overlays, 0xA5, sizeof overlays);
    for (int i = 0; i < MAX_STR_VARIABLES; i++) { strvars[i].str = sbuf[i]; strvars[i].maxlen = 32; }
}

static int n_faelle = 0, fails = 0, n_ign = 0, n_ok = 0, n_kurz = 0, n_v = 0;
static unsigned long expect_cnt = 0;            /* unabhaengig mitgezaehlt */
static void fehl (const char * fall, const char * was, const std::string & got = "")
{
    fails++;
    if (fails <= 40) printf ("      FEHLER [%s]: %s%s%s\n", fall, was, got.empty () ? "" : " -- bekam: ", got.c_str ());
}

/* art: 'J' = weist zu, 'I' = still entfallen (zaehlt), 'V' = Rahmenzeile (zaehlt nicht), 'K' = zu kurz (!v, zaehlt nicht) */
static void fall (const char * line, char art, const char * code, const char * idx, bool zeit_vor = true)
{
    char buf[160]; snprintf (buf, sizeof buf, "%s", line);
    n_faelle++;
    if (zeit_vor) fake_ms += 60000;                     /* Drossel offen: jede Zeile darf melden */
    fill (); Snap a = snap ();
    Serial.out.clear (); size_t lr = logring.size ();
    unsigned int rc = var_set_parameter (buf);
    Snap b = snap ();
    bool geaendert = a.b != b.b;
    std::string ser = Serial.out;
    std::vector<std::string> neu (logring.begin () + lr, logring.end ());
    std::vector<std::string> ign; for (auto & l : neu) if (l.find ("var ignoriert") != std::string::npos) ign.push_back (l);

    unsigned int rc_soll = art == 'K' ? 0 : 1;
    if (rc != rc_soll) fehl (line, "Rueckgabe veraendert");
    if (art == 'J' && ! geaendert) fehl (line, "gueltige Zeile hat nichts zugewiesen (Tabelle falsch?)");
    if (art != 'J' && geaendert) fehl (line, "Zeile sollte nichts zuweisen (Tabelle falsch?)");

    if (art == 'I')
    {
        n_ign++; expect_cnt++;
        char soll[96]; snprintf (soll, sizeof soll, "- var ignoriert #%lu code=%s idx=%s", expect_cnt, code, idx);
        if (ign.size () != 1 || ign[0] != soll) fehl (line, (std::string ("API-Ring: erwartet \"") + soll + "\"").c_str (), ign.empty () ? "(nichts)" : ign[0]);
        if (ser != std::string (soll) + "\r\n") fehl (line, (std::string ("Mitschnitt: erwartet \"") + soll + "\"").c_str (), ser.empty () ? "(nichts)" : ser);
    }
    else
    {
        if (art == 'J') n_ok++; else if (art == 'V') n_v++; else n_kurz++;
        if (! ign.empty () || ser.find ("var ignoriert") != std::string::npos) fehl (line, "darf nicht als ignoriert melden", ign.empty () ? ser : ign[0]);
        if (art != 'K' && ! ser.empty ()) fehl (line, "unerwartete Ausgabe auf der Bruecke", ser);
    }
}

static std::string hx (unsigned v) { char b[8]; snprintf (b, sizeof b, "%02x", v & 0xFF); return b; }
static std::string L (const char * pre, unsigned idx, const char * rest) { return std::string (pre) + hx (idx) + rest; }

int main (void)
{
    printf ("a35 (%s)\n", BEZEICHNUNG);
    /* ---- weist zu (J): je eine gueltige Zeile je Zuweisungsstelle, 29 Stellen ---- */
    struct { std::string l; } J[] = {
        { L ("N", 0x01, "3412") },
        { L ("n", DISPLAY_DIMMED_DISPLAY_COLORS, "0f0f") }, { L ("n", DISPLAY_DIMMED_AMBILIGHT_COLORS, "000f") },
        { L ("S", 0x00, "Hallo") },
        { L ("T", 0x00, "20261012123456") },
        { L ("DC", 0x00, "01020304") },
        { L ("AN", 0x00, "Name") }, { L ("AD", 0x01, "03") }, { L ("AE", 0x02, "04") }, { L ("AF", 0x03, "05") },
        { L ("CN", 0x00, "Name") }, { L ("CD", 0x00, "03") }, { L ("CE", 0x00, "04") }, { L ("CF", 0x00, "05") },
        { L ("MN", 0x00, "Name") }, { L ("MD", 0x00, "03") }, { L ("ME", 0x00, "04") }, { L ("MF", 0x00, "05") },
        { L ("OT", 0x00, "02") }, { L ("OI", 0x00, "03") }, { L ("OD", 0x00, "04") }, { L ("OC", 0x00, "05") },
        { L ("OS", 0x00, "0102") }, { L ("OY", 0x00, "06") }, { L ("ON", 0x00, "Text") }, { L ("OF", 0x00, "07") },
        { L ("t", 0x00, "1e0001") }, { L ("a", 0x00, "1e0001") }, { L ("l", 0x00, "1e0001") },
        { L ("I", 0x00, "0112343456") + "78" },
    };
    for (auto & j : J) fall (j.l.c_str (), 'J', "", "");

    /* ---- still entfallen (I): Index >= MAX je Stelle, Wert/Element ausser Bereich, unbekannte Buchstaben ---- */
    struct { std::string l; const char * code; unsigned idx; } I[] = {
        { L ("N", MAX_NUM_VARIABLES, "3412"), "N", MAX_NUM_VARIABLES },
        { L ("N", 0xff, "3412"), "N", 0xff },
        { L ("n", DISPLAY_DIMMED_DISPLAY_COLORS, "100f"), "n", DISPLAY_DIMMED_DISPLAY_COLORS },     /* Element 16 */
        { L ("n", DISPLAY_DIMMED_DISPLAY_COLORS, "0010"), "n", DISPLAY_DIMMED_DISPLAY_COLORS },     /* Wert 16 > MAX_BRIGHTNESS */
        { L ("n", DISPLAY_DIMMED_AMBILIGHT_COLORS, "00ff"), "n", DISPLAY_DIMMED_AMBILIGHT_COLORS },
        { L ("n", DISPLAY_DIMMED_AMBILIGHT_COLORS, "ff00"), "n", DISPLAY_DIMMED_AMBILIGHT_COLORS },
        { L ("n", MAX_NUM8_ARRAYS, "0001"), "n", MAX_NUM8_ARRAYS },                                 /* unbekanntes Feld */
        { L ("S", MAX_STR_VARIABLES, "geheim"), "S", MAX_STR_VARIABLES },
        { L ("T", MAX_TM_VARIABLES, "20261012123456"), "T", MAX_TM_VARIABLES },
        { L ("DC", MAX_DSP_COLOR_VARIABLES, "01020304"), "DC", MAX_DSP_COLOR_VARIABLES },
        { L ("DX", 0x00, "01020304"), "DX", 0 },
        { L ("AN", MAX_DISPLAY_ANIMATION_VARIABLES, "Name"), "AN", MAX_DISPLAY_ANIMATION_VARIABLES },
        { L ("AD", MAX_DISPLAY_ANIMATION_VARIABLES, "03"), "AD", MAX_DISPLAY_ANIMATION_VARIABLES },
        { L ("AE", 0xff, "03"), "AE", 0xff }, { L ("AF", 0x40, "03"), "AF", 0x40 }, { L ("AX", 0x00, "03"), "AX", 0 },
        { L ("CN", MAX_COLOR_ANIMATION_VARIABLES, "Name"), "CN", MAX_COLOR_ANIMATION_VARIABLES },
        { L ("CD", MAX_COLOR_ANIMATION_VARIABLES, "03"), "CD", MAX_COLOR_ANIMATION_VARIABLES },
        { L ("CE", 0xff, "03"), "CE", 0xff }, { L ("CF", 0xfe, "03"), "CF", 0xfe }, { L ("CX", 0x00, "03"), "CX", 0 },
        { L ("MN", MAX_AMBILIGHT_MODE_VARIABLES, "Name"), "MN", MAX_AMBILIGHT_MODE_VARIABLES },
        { L ("MD", MAX_AMBILIGHT_MODE_VARIABLES, "03"), "MD", MAX_AMBILIGHT_MODE_VARIABLES },
        { L ("ME", 0xff, "03"), "ME", 0xff }, { L ("MF", 0x80, "03"), "MF", 0x80 }, { L ("MX", 0x00, "03"), "MX", 0 },
        { L ("OT", MAX_OVERLAYS, "02"), "OT", MAX_OVERLAYS }, { L ("OI", MAX_OVERLAYS, "02"), "OI", MAX_OVERLAYS },
        { L ("OD", MAX_OVERLAYS, "02"), "OD", MAX_OVERLAYS }, { L ("OC", MAX_OVERLAYS, "02"), "OC", MAX_OVERLAYS },
        { L ("OS", MAX_OVERLAYS, "0102"), "OS", MAX_OVERLAYS }, { L ("OY", MAX_OVERLAYS, "02"), "OY", MAX_OVERLAYS },
        { L ("ON", MAX_OVERLAYS, "Text"), "ON", MAX_OVERLAYS }, { L ("OF", 0xff, "02"), "OF", 0xff },
        { L ("OX", 0x00, "02"), "OX", 0 },
        { L ("t", MAX_NIGHT_TIME_VARIABLES, "1e0001"), "t", MAX_NIGHT_TIME_VARIABLES },
        { L ("a", MAX_NIGHT_TIME_VARIABLES, "1e0001"), "a", MAX_NIGHT_TIME_VARIABLES },
        { L ("l", MAX_ALARM_TIME_VARIABLES, "1e0001"), "l", MAX_ALARM_TIME_VARIABLES },
        { L ("I", MAX_IR_CODES, "011234345678"), "I", MAX_IR_CODES },
    };
    for (auto & i : I) fall (i.l.c_str (), 'I', i.code, hx (i.idx).c_str ());
    /* unbekannte Kommandobuchstaben: idx "--", nicht druckbare Zeichen als '?' */
    fall ("Z0102", 'I', "Z", "--"); fall ("x", 'I', "x", "--"); fall ("\x01" "abc", 'I', "?", "--");
    fall ("~~", 'I', "~", "--"); fall (" 12", 'I', "?", "--");

    /* ---- Rahmenzeilen V: zaehlen nicht ---- */
    fall ("VB0301", 'V', "", ""); fall ("VE01", 'V', "", ""); fall ("VQ", 'V', "", ""); fall ("V", 'V', "", "");

    /* ---- zu kurz: "!v"-Pfad (var_cmd_reject), zaehlt nicht als still ---- */
    fall ("N01", 'K', "", ""); fall ("DC00", 'K', "", ""); fall ("T00", 'K', "", ""); fall ("", 'K', "", "");

    /* ---- A20: kein Wert in der Zeile ---- */
    for (auto & l : logring) if (l.find ("geheim") != std::string::npos && l.find ("var ignoriert") != std::string::npos)
        fehl ("A20", "Wert in der Zeile", l);

    /* ---- Drosselung: erste sofort, danach hoechstens eine je 60 s, Zaehler laeuft weiter ---- */
    int vor = fails;
    {
        auto count_ign = [] () { int c = 0; for (auto & l : logring) if (l.find ("var ignoriert") != std::string::npos) c++; return c; };
        char z[16];
        fake_ms += 60000;
        int c0 = count_ign ();
        strcpy (z, "Zff"); var_set_parameter (z); expect_cnt++;              /* meldet */
        int c1 = count_ign ();
        for (int k = 0; k < 10; k++) { fake_ms += 5000; strcpy (z, "Zff"); var_set_parameter (z); expect_cnt++; }   /* +50 s: still */
        int c2 = count_ign ();
        fake_ms += 9999; strcpy (z, "Zff"); var_set_parameter (z); expect_cnt++;   /* 59'999 ms nach der Meldung: still */
        int c3 = count_ign ();
        fake_ms += 1; Serial.out.clear (); strcpy (z, "Zff"); var_set_parameter (z); expect_cnt++;  /* 60'000 ms: meldet */
        int c4 = count_ign ();
        char soll[96]; snprintf (soll, sizeof soll, "- var ignoriert #%lu code=Z idx=--", expect_cnt);
        n_faelle += 4;
        if (c1 - c0 != 1) fehl ("Drossel", "erste Zeile nicht sofort");
        if (c2 != c1)     fehl ("Drossel", "innerhalb 60 s gemeldet");
        if (c3 != c2)     fehl ("Drossel", "bei 59'999 ms gemeldet");
        if (c4 != c3 + 1 || logring.back () != soll) fehl ("Drossel", (std::string ("nach 60 s: erwartet ") + soll).c_str (), logring.back ());
        /* Ueberlauf der Millisekundenuhr (L256): Meldung kurz vor 2^32, naechste 60 s spaeter jenseits der Grenze */
        fake_ms = 0xFFFFFFFFu - 20000u; strcpy (z, "Zff"); var_set_parameter (z); expect_cnt++;
        int c5 = count_ign ();
        fake_ms += 30000; strcpy (z, "Zff"); var_set_parameter (z); expect_cnt++;   /* 30 s spaeter, gewickelt: still */
        int c6 = count_ign ();
        fake_ms += 30000; strcpy (z, "Zff"); var_set_parameter (z); expect_cnt++;   /* 60 s: meldet */
        int c7 = count_ign ();
        n_faelle += 3;
        if (c5 != c4 + 1) fehl ("Wickeln", "Meldung vor der Grenze fehlt");
        if (c6 != c5)     fehl ("Wickeln", "30 s nach der Meldung (ueber die Grenze) gemeldet");
        if (c7 != c6 + 1) fehl ("Wickeln", "60 s nach der Meldung (ueber die Grenze) nicht gemeldet");
    }
    int drossel_fehl = fails - vor;

    printf ("  Faelle: %d (zuweisend %d, still entfallen %d, Rahmen V %d, zu kurz %d, Drossel 7)\n", n_faelle, n_ok, n_ign, n_v, n_kurz);
    printf ("  Zaehlstand erwartet #%lu\n", expect_cnt);
    if (drossel_fehl) printf ("  Drossel: %d Fehler\n", drossel_fehl);
    printf ("%s  a35 %s: %d Fehler\n", fails ? "FEHL" : "OK  ", BEZEICHNUNG, fails);
    return fails ? 1 : 0;
}
