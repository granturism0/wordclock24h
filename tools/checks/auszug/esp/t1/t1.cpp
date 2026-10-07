/* Pruefstand S.1-S.3 (Runde S, ESP-Seite): Abgleichrahmen, Markenpflicht, Zuordnungszeile.
 * Uebersetzt den ECHTEN Code aus ESP-uclock.ino (gen.py), mit Zieltypen:
 *   uint_fast8_t/uint_fast16_t -> unsigned int (xtensa-lx106-elf-gcc: __UINT_FAST8_TYPE__ unsigned int)
 *   Uhrvariablen -> uint32_t (L256), millis() -> uint32_t
 * Jeder Fall laeuft in einem eigenen Prozess (static-Zustand von var_sync_check()).
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <vector>
#include "pruefstand.h"
#define uint_fast8_t  unsigned int
#define uint_fast16_t unsigned int
#include VARSHDR
#define STM32_LOG_LINE_LEN  120

static zp_millis fake_ms = 0;
static zp_millis millis (void) { return fake_ms; }
struct SerialStub {
    std::string out;
    void println (const char * s) { out += s; out += "\r\n"; }
    void print (const char * s) { out += s; }
    void flush (void) {}
} Serial;
static std::vector<std::string> logring;
static void stm32_log_append (const char * l) { logring.push_back (l); }
char         wifi_ip_address[20];
unsigned int numvars[MAX_NUM_VARIABLES];
static std::vector<std::string> applied;
uint_fast8_t var_set_parameter (char * p) { applied.push_back (p); return 1; }

#include "bruecke_code.inc"

/* Unabhaengige Pruefsumme nach Definition (Laenge als Startwert, Fletcher-artig). */
static uint16_t ref_crc (const char * s)
{
    uint8_t s1 = (uint8_t) strlen (s), s2 = 0;
    for (const char * p = s; *p; p++) { s1 = (uint8_t) (s1 + (uint8_t) *p); s2 = (uint8_t) (s2 + s1); }
    return (uint16_t) ((s2 << 8) | s1);
}
static std::string marked (const char * payload)
{
    char b[160]; snprintf (b, sizeof b, "var %s*%04x", payload, ref_crc (payload)); return b;
}
static std::string send (std::string line)          /* eine Zeile an den ESP, Antwort zurueck */
{
    char buf[160]; Serial.out.clear (); snprintf (buf, sizeof buf, "%s", line.c_str ());
    handle_line (buf); return Serial.out;
}
static int fails = 0;
static void expect (bool ok, const char * what, const std::string & got = "")
{
    if (! ok) { fails++; printf ("      FEHLER: %s%s%s\n", what, got.empty () ? "" : " -- bekam: ", got.c_str ()); }
}
static bool log_has (const char * needle)
{
    for (auto & l : logring) if (strstr (l.c_str (), needle)) return true;
    return false;
}
static std::string esc (const std::string & s)
{
    std::string r; for (char c : s) { if (c == '\r') r += "\\r"; else if (c == '\n') r += "\\n"; else r += c; } return r;
}
static std::string ack_for (const char * payload)
{
    char b[16]; snprintf (b, sizeof b, "ACK %02x\r\n", ref_crc (payload) >> 8); return b;
}

/* ---------------------------------------------------------------- Faelle */
static void f_punkt_mit_zuordnung (void)        /* AKS.4: ACK vor dem Punkt, Punkt nackt */
{
    std::string r = send (marked ("N1d0100"));
    expect (r == ack_for ("N1d0100") + ".\r\n", "markierte Zeile => \"ACK <hoeheres Byte>\" + nackter Punkt", esc (r));
}
static void f_nak_mit_empfangener_marke (void)  /* !v bei falscher Pruefsumme, ACK nennt die EMPFANGENE Marke */
{
    std::string r = send ("var N1d0100*beef");
    expect (r == "ACK be\r\n!v\r\n", "falsche Marke => \"ACK be\" + \"!v\"", esc (r));
    expect (applied.empty (), "falsch markierte Zeile darf nicht angewandt werden");
}
static void f_unmarkiert_ohne_erwartung (void)  /* Rueckfall: kein Zaehler, Punkt, ACK -- */
{
    std::string r = send ("var N1dffff");
    expect (r == "ACK --\r\n.\r\n", "unmarkiert ohne Erwartung => \"ACK --\" + Punkt", esc (r));
    expect (! log_has ("unmarkiert"), "ohne Erwartung kein Zaehlereintrag");
    expect (applied.size () == 1, "unmarkierte Zeile wird wie bisher angewandt");
}
static void f_zaehler_nach_marke (void)         /* AKS.7: unmarkiert nach erster Marke => Zaehler im Logring */
{
    send (marked ("N0000"));
    std::string r = send ("var N0201ff");
    expect (r == "ACK --\r\n.\r\n", "gewoehnliche unmarkierte Zeile nach Marke => angewandt, Punkt", esc (r));
    expect (log_has ("- var unmarkiert #1 len=7"), "Logring: \"- var unmarkiert #1 len=7\"");
}
static void f_pflicht_hw (void)                 /* AKS.8: HARDWARE_CONFIGURATION ohne Marke => !v, nicht angewandt */
{
    send (marked ("N0000")); applied.clear ();
    std::string r = send ("var N1d0f00");
    expect (r == "ACK --\r\n!v\r\n", "HW-Konfiguration ohne Marke => \"!v\"", esc (r));
    expect (applied.empty (), "Pflichtzeile ohne Marke darf nicht angewandt werden");
    expect (log_has ("- var ohne Marke abgewiesen #1"), "Abweisung im Logring");
}
static void f_pflicht_host_pfad (void)          /* Update-Host (09) und -Pfad (0a) Pflicht, 0b nicht */
{
    send (marked ("N0000")); applied.clear ();
    std::string a = send ("var S09evil.example");
    std::string b = send ("var S0a/x/");
    std::string c = send ("var S0b%d.%m.");
    expect (a == "ACK --\r\n!v\r\n", "Update-Host ohne Marke => \"!v\"", esc (a));
    expect (b == "ACK --\r\n!v\r\n", "Update-Pfad ohne Marke => \"!v\"", esc (b));
    expect (c == "ACK --\r\n.\r\n", "Index 0b (nicht in der Liste) => Punkt", esc (c));
    expect (applied.size () == 1, "nur die nicht pflichtige Zeile angewandt");
    std::string d = send (marked ("S09mein.host"));
    expect (d == ack_for ("S09mein.host") + ".\r\n", "Update-Host MIT Marke => angenommen", esc (d));
}
static void f_pflicht_im_abgleich (void)        /* Eroeffnung mit Flag 0x01 => Pflicht sofort, schon fuer das 3. Kommando */
{
    std::string r0 = send ("var VB0301");       /* Eroeffnung selbst unmarkiert: Sitzung hat noch keine Marke */
    expect (r0 == "ACK --\r\n.\r\n", "Eroeffnung wird quittiert", esc (r0));
    std::string r = send ("var N1d0f00");
    expect (r == "ACK --\r\n!v\r\n", "im markierten Abgleich: HW-Konfiguration ohne Marke => \"!v\"", esc (r));
}
static void f_abgleich_ohne_markenflag (void)   /* Eroeffnung ohne 0x01 loescht die Sitzungserwartung: kein Dauerzustand */
{
    send (marked ("N0000"));
    send (marked ("VB0207"));
    std::string r = send ("var N1d0f00");
    expect (r == "ACK --\r\n.\r\n", "STM meldet \"ich markiere nicht\" => Pflicht aufgehoben", esc (r));
}
static void f_erwartung_endet_mit_abschluss (void)
{
    send ("var VB0305"); send ("var VE05");
    std::string r = send ("var N1d0f00");
    expect (r == "ACK --\r\n.\r\n", "nach der Abschlussmarke IHRES Abgleichs endet die Erwartung", esc (r));
    expect (log_has ("Abschlussmarke 05, unmarkiert 1,"), "Abschlusszeile mit Zaehlerstand im Logring (die unmarkierte Abschlusszeile selbst zaehlt)");
}
static void f_erwartung_endet_nach_frist (void)
{
    fake_ms = 0xFFFFF000u;                      /* ueber den Wortrand (L256) */
    send ("var VB0305");
    fake_ms += 31000;
    std::string r = send ("var N1d0f00");
    expect (r == "ACK --\r\n.\r\n", "ohne Abschlussmarke endet die Erwartung nach 30 s (ueber den Wortrand)", esc (r));
}
static void f_erwartung_haelt_ueber_wortrand (void)
{
    fake_ms = 0xFFFFF000u;                      /* Eroeffnung kurz vor dem Wortrand, 10 s spaeter danach */
    send ("var VB0305");
    fake_ms += 10000;
    std::string r = send ("var N1d0f00");
    expect (r == "ACK --\r\n!v\r\n", "10 s nach der Eroeffnung, ueber den Wortrand: Erwartung gilt noch", esc (r));
}
static void f_rahmen_verstuemmelt (void)
{
    std::string r = send ("var VBz301");
    expect (r == "ACK --\r\n!v\r\n", "verstuemmelte Eroeffnung => \"!v\"", esc (r));
    std::string u = send ("var VX");
    expect (u == "ACK --\r\n.\r\n", "unbekannter Unterbuchstabe => Punkt, keine Nachsendeschleife", esc (u));
    expect (applied.empty (), "Rahmenzeilen gehen nie an var_set_parameter()");
}
/* var_sync_check(): Ablauf ueber die Zeit */
static void tick (zp_millis ms) { fake_ms += ms; var_sync_check (); }
static int syncvars (void) { int n = 0; size_t p = 0; while ((p = Serial.out.find ("SYNCVARS", p)) != std::string::npos) { n++; p++; } return n; }
static void start_ip (void) { strcpy (wifi_ip_address, "192.0.2.2"); numvars[HARDWARE_CONFIGURATION_NUM_VAR] = 0xFFFF; Serial.out.clear (); var_sync_check (); }

static void f_sync_wartet_auf_marke (void)      /* AKS.3: Erfolgszeile erst NACH der Abschlussmarke */
{
    start_ip ();
    send (marked ("VB0311"));
    numvars[HARDWARE_CONFIGURATION_NUM_VAR] = 0x0123;   /* 3. Kommando ist da */
    Serial.out.clear (); tick (10000);
    expect (! log_has ("Variablensatz vollstaendig"), "vor der Abschlussmarke KEINE Erfolgszeile (HW-Konfiguration allein reicht nicht)");
    expect (syncvars () == 0, "laufender Abgleich wird nicht ein zweites Mal angefordert");
    send (marked ("VE11"));
    Serial.out.clear (); tick (10000);
    expect (log_has ("Variablensatz vollstaendig, keine Anforderung noetig (Abschlussmarke)"), "Erfolgszeile nach der Marke");
}
static void f_sync_rueckfall_alter_stm (void)   /* AKS.6: keine Eroeffnung => bisheriges Kriterium */
{
    start_ip ();
    numvars[HARDWARE_CONFIGURATION_NUM_VAR] = 0x0123;
    Serial.out.clear (); tick (10000);
    expect (log_has ("Variablensatz vollstaendig, keine Anforderung noetig"), "ohne Eroeffnung: Erfolg bei HW != 0xFFFF");
    expect (syncvars () == 0, "ohne Eroeffnung: keine Anforderung");
}
static void f_sync_marke_bleibt_aus (void)
{
    start_ip ();
    send (marked ("VB0312"));
    numvars[HARDWARE_CONFIGURATION_NUM_VAR] = 0x0123;
    Serial.out.clear ();
    tick (10000); tick (10000); tick (10000);
    expect (syncvars () == 1, "Marke bleibt 30 s aus => genau eine Anforderung", std::to_string (syncvars ()));
    expect (log_has ("unvollstaendig (Abschlussmarke fehlt)"), "Grund im Logring");
    expect (! log_has ("vollstaendig, keine"), "keine Erfolgszeile ohne Marke");
}
static void f_nachgesendete_eroeffnung (void)   /* A32 sendet eine Eroeffnung nach, nachdem der Abgleich durch ist */
{
    start_ip ();
    send (marked ("VB0313")); send (marked ("VE13")); send (marked ("VB0313"));
    numvars[HARDWARE_CONFIGURATION_NUM_VAR] = 0x0123;
    Serial.out.clear (); tick (10000);
    expect (log_has ("vollstaendig, keine Anforderung noetig (Abschlussmarke)"), "nachgesendete Eroeffnung oeffnet den Abgleich nicht neu");
}
/* ---- Review S.6 ---- */
static void f_h1_cap_verlust_host (void)        /* H1: Marke aus der Sitzung, dann markiert der STM nicht mehr; nur Host kommt */
{
    send (marked ("N0000"));                    /* faellt ins Setup-Fenster: var_mark_seen = 1 */
    applied.clear ();
    std::string a = send ("var S09mein.host");  /* STM hat cap_var_crc = 0 gelatcht: unmarkiert, dreimal */
    std::string b = send ("var S09mein.host");
    std::string c = send ("var S09mein.host");
    expect (c == "ACK --\r\n.\r\n", "dritte Sendung des Update-Hosts wird angenommen (STM sendet hoechstens dreimal)", esc (a) + " | " + esc (b) + " | " + esc (c));
    expect (applied.size () == 1, "Update-Host genau einmal angewandt", std::to_string (applied.size ()));
    expect (log_has ("STM markiert nicht mehr"), "Aufloesung steht im Logring");
    std::string d = send ("var N1d0f00");
    expect (d == "ACK --\r\n.\r\n", "danach HW-Konfiguration unmarkiert angenommen -- kein Dauerzustand", esc (d));
}
static void f_h1_cap_verlust_abgleich (void)    /* H1: Vollabgleich eines alten STM (keine Eroeffnung), HW ist das 3. Kommando */
{
    send (marked ("N0000")); applied.clear ();
    send ("var N0001ff"); send ("var N0100ff");
    std::string r = send ("var N1d0f00");
    expect (r == "ACK --\r\n.\r\n", "HW-Konfiguration als 3. unmarkierte Zeile in Folge wird angenommen", esc (r));
}
static void f_h1_gegenrichtung (void)           /* markierender STM, einzelne beschaedigte Zeilen: Erwartung bleibt */
{
    send (marked ("N0000"));
    std::string a = send ("var N1d0f00");       /* beschaedigt, Marke verloren */
    send (marked ("N1d0f00"));                  /* Nachsendung, markiert */
    send ("var N0201ff");                       /* wieder eine beschaedigte, gewoehnliche */
    send (marked ("N0300ff"));
    send ("var N0401ff");
    std::string b = send ("var N1d0f00");       /* zwei in Folge, dann Pflicht */
    expect (a == "ACK --\r\n!v\r\n" && b == "ACK --\r\n!v\r\n", "einzelne Schaeden: Pflicht greift weiter", esc (a) + " | " + esc (b));
    expect (! log_has ("STM markiert nicht mehr"), "keine Aufloesung bei markierendem STM");
    send ("var VB0301");                        /* Erwartung im Abgleich bleibt von H1 unberuehrt */
}
static void f_m1_phantom (void)                 /* M1: Marke ohne Eroeffnung, danach nachgesendete Eroeffnung derselben Nummer */
{
    start_ip ();
    send (marked ("VE13"));
    send (marked ("VB0313"));
    numvars[HARDWARE_CONFIGURATION_NUM_VAR] = 0x0123;
    Serial.out.clear (); tick (10000);
    expect (log_has ("vollstaendig, keine Anforderung noetig"), "kein Phantom-Abgleich: Erfolg beim ersten Pruefen");
    expect (syncvars () == 0, "keine Anforderung");
}
static void f_m2_index_wie_parser (void)        /* M2: Pflicht sieht dasselbe Ziel wie htoi() im Parser */
{
#ifdef HAVE_VAR_MARK_REQUIRED
    int abw = 0, n = 0; char p[8];
    for (int c1 = 0x21; c1 < 0x7f; c1++) for (int c2 = 0x21; c2 < 0x7f; c2++)
    {
        p[1] = (char) c1; p[2] = (char) c2; p[3] = 'x'; p[4] = 0;
        unsigned t = htoi (p + 1, 2);
        p[0] = 'S'; abw += var_mark_required (p) != (t == UPDATE_HOST_VAR || t == UPDATE_PATH_VAR); n++;
        p[0] = 'N'; abw += var_mark_required (p) != (t == HARDWARE_CONFIGURATION_NUM_VAR); n++;
    }
    expect (abw == 0, "erschoepfend ueber 2 x 94 x 94 Indexpaare: Pflicht == Ziel des Parsers", std::to_string (abw) + " Abweichungen von " + std::to_string (n));
#else
    expect (false, "var_mark_required() fehlt");
#endif
    send (marked ("N0000"));
    std::string a = send ("var S?9evil");
    std::string b = send ("var S0?x");
    std::string c = send ("var N1?0f00");
    expect (a == "ACK --\r\n!v\r\n", "S?9 (Parser: Update-Host) => Pflicht, \"!v\"", esc (a));
    expect (b == "ACK --\r\n.\r\n", "S0? (Parser: Index 0) => keine Pflicht", esc (b));
    expect (c == "ACK --\r\n.\r\n", "N1? (Parser: Index 0x10) => keine Pflicht", esc (c));
}
static void f_summen_byte (void)                /* Begruendung fuer das HOEHERE Byte */
{
    uint16_t a = ref_crc ("N1d0100"), b = ref_crc ("N1e0000");
    expect ((a & 0xff) == (b & 0xff) && (a >> 8) != (b >> 8), "niedriges Byte gleich, hoeheres verschieden");
    expect (var_crc ("N1d0100", 7) == a, "eigene Rechnung des ESP stimmt mit der Referenz");
}

struct Fall { const char * name; void (*fn) (void); };
static Fall faelle[] = {
    { "S.2 Punkt mit Zuordnung (AKS.4)",               f_punkt_mit_zuordnung },
    { "S.2 !v mit empfangener Marke",                  f_nak_mit_empfangener_marke },
    { "S.2 unmarkiert ohne Erwartung (Rueckfall)",     f_unmarkiert_ohne_erwartung },
    { "S.3 Zaehler nach erster Marke (AKS.7)",         f_zaehler_nach_marke },
    { "S.3 Pflicht HW-Konfiguration (AKS.8)",          f_pflicht_hw },
    { "S.3 Pflicht Update-Host/-Pfad, nur diese",      f_pflicht_host_pfad },
    { "S.3 Pflicht im markierten Abgleich sofort",     f_pflicht_im_abgleich },
    { "S.3 Eroeffnung ohne Markenflag hebt auf",       f_abgleich_ohne_markenflag },
    { "S.3 Erwartung endet mit Abschlussmarke",        f_erwartung_endet_mit_abschluss },
    { "S.3 Erwartung endet nach Frist (Wortrand)",     f_erwartung_endet_nach_frist },
    { "S.3 Erwartung haelt ueber den Wortrand (L256)", f_erwartung_haelt_ueber_wortrand },
    { "S.1 Rahmenzeile verstuemmelt/unbekannt",        f_rahmen_verstuemmelt },
    { "S.1 Erfolg erst nach Marke (AKS.3)",            f_sync_wartet_auf_marke },
    { "S.1 Rueckfall ohne Eroeffnung (AKS.6)",         f_sync_rueckfall_alter_stm },
    { "S.1 Marke bleibt aus",                          f_sync_marke_bleibt_aus },
    { "S.1 nachgesendete Eroeffnung",                  f_nachgesendete_eroeffnung },
    { "S.2 Wahl des hoeheren Bytes",                   f_summen_byte },
    { "H1 CAP-Verlust, nur Update-Host",               f_h1_cap_verlust_host },
    { "H1 CAP-Verlust, Abgleich eines alten STM",      f_h1_cap_verlust_abgleich },
    { "H1 Gegenrichtung: einzelne Schaeden",           f_h1_gegenrichtung },
    { "M1 kein Phantom-Abgleich",                      f_m1_phantom },
    { "M2 Index wie der Parser (S?9, S0?, N1?)",       f_m2_index_wie_parser },
};

int main (void)
{
    int n = sizeof faelle / sizeof faelle[0], bad = 0;
    printf ("Pruefstand S.1-S.3 (%s), %d Faelle\n", BEZEICHNUNG, n);
    for (int i = 0; i < n; i++)
    {
        fflush (stdout);
        pid_t pid = fork ();
        if (pid == 0) { faelle[i].fn (); fflush (stdout); _exit (fails ? 1 : 0); }
        int st; waitpid (pid, &st, 0);
        int ok = WIFEXITED (st) && WEXITSTATUS (st) == 0;
        printf ("  [%s] %s%s\n", ok ? " ok " : "FEHL", faelle[i].name, WIFSIGNALED (st) ? " (abgestuerzt)" : "");
        bad += ! ok;
    }
    printf ("Ergebnis: %d von %d Faellen bestanden\n", n - bad, n);
    return bad ? 1 : 0;
}
