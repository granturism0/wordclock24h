/* Pruefstand C55/C44 (Paket 2026-10-10, E.2): Datumsformat und Koordinaten an ALLEN ESP-Schreibwegen.
 *
 * Teil T  Fall-Tabelle regeln.tsv gegen die echten Schreibwege: /api/date_ticker_format_set und Legacy
 *         savedtf (Feld datumsformat), /api/weather_coordinates_set und Legacy savelonlat (Felder laenge,
 *         breite - die jeweils andere Koordinate gueltig). Je Fall: angenommen genau dann, wenn die Tabelle
 *         "gueltig" sagt, auf BEIDEN Wegen; angenommen heisst gespeichert (set_strvar), mit Komma als Punkt.
 * Teil P  Paar- und Leerstandsregeln (bestehend, L48) samt Fehlerkennung und Meldung, API und Legacy.
 * Teil O  (nur neu) erschoepfend gegen ein unabhaengig formuliertes Orakel (std::regex + strtod):
 *         Datumsformat alle Zeichenketten bis Laenge 5 ueber "dDmMyY./- x<", Koordinaten alle bis Laenge 6
 *         ueber "-0189.,+e ", dazu ein Zahlengitter -181..181 in Tausendstel und Zufallsketten Laenge 7-9.
 *
 * Gegenprobe (DIR-014): Der Ausgangsstand nimmt <"& und <scri an (T) => FEHL. Teil O laesst sich dort
 * nicht uebersetzen (die Prueffunktionen fehlen) und wird AUSDRUECKLICH als ausgelassen gemeldet.
 * Typbreiten (L256): http_coordinate_ok () rechnet in long - auf dem ESP 32 Bit, hier 64. Der ganzzahlige
 * Teil hat nach der Laengenpruefung hoechstens acht Ziffern (99'999'999 < 2^31), ein Ueberlauf ist also auf
 * keiner der beiden Breiten moeglich; das Ergebnis haengt nicht an der Breite. Laengen sind < 2^16.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include VARSHDR

/* ---- Attrappen ---- */
static std::string                          vals[MAX_STR_VARIABLES];
static std::vector<std::pair<int, std::string>> sets;
STR_VAR * get_strvar (STR_VARIABLE v)
{
    static char b[MAX_STR_VARIABLES][128]; static STR_VAR s[MAX_STR_VARIABLES];
    snprintf (b[v], sizeof b[v], "%s", vals[v].c_str ()); s[v].str = b[v]; s[v].maxlen = 127; return &s[v];
}
unsigned int set_strvar (STR_VARIABLE v, const char * x) { sets.push_back ({ (int) v, x }); vals[v] = x; return 1; }

static std::map<std::string, std::string> params;
static char * http_get_param (const char * n)
{
    static char b[8][300]; static int i; char * p = b[i++ % 8];
    snprintf (p, sizeof b[0], "%s", params.count (n) ? params[n].c_str () : ""); return p;
}
static int          last_error;
static std::string  last_detail;
static int          oks;
static void http_json_error (unsigned code, const char * d) { last_error = (int) code; last_detail = d; }
static void http_json_ok (void) { oks++; }

#include "regeln_code.inc"
#include "regeln_neu.inc"
#include "regeln_api.inc"

static void reset (void) { sets.clear (); last_error = 0; last_detail.clear (); oks = 0; }

/* Legacy-Zweige: der ausgeschnittene Block in einer Funktion mit den lokalen Variablen der Seite */
static const char * leg_alert;
static const char * leg_message;
static void legacy_dtf (void)
{
    char *          action = (char *) "savedtf";
    const char *    message = (const char *) 0;
    const char *    alert_message = (const char *) 0;
    if (0) {}
#include "regeln_dtf.inc"
    leg_alert = alert_message; leg_message = message;
}
static void legacy_lonlat (void)
{
    char *          action = (char *) "savelonlat";
    const char *    message = (const char *) 0;
    const char *    alert_message = (const char *) 0;
    if (0) {}
#include "regeln_lonlat.inc"
    leg_alert = alert_message; leg_message = message;
}

static int n, fails;
static void fehl (const char * teil, int fall, const std::string & was) { fails++; printf ("FEHL %s fall %d: %s\n", teil, fall, was.c_str ()); }
static std::string komma_zu_punkt (std::string s) { for (auto & c : s) if (c == ',') c = '.'; return s; }

/* Ergebnis eines Schreibwegs: gespeichert? Welche Werte? */
struct Erg { bool gespeichert; std::string a, b; int code; std::string detail; };

static Erg api_dtf (const std::string & v)
{
    reset (); params = { { "value", v } }; vals[DATE_TICKER_FORMAT_VAR] = "D.M.Y";
    http_api_date_ticker_format_set ();
    Erg e { false, "", "", last_error, last_detail };
    for (auto & s : sets) if (s.first == DATE_TICKER_FORMAT_VAR) { e.gespeichert = true; e.a = s.second; }
    if (e.gespeichert && (oks != 1 || last_error)) e.code = -1;
    return e;
}
static Erg leg_dtf (const std::string & v)
{
    reset (); params = { { "action", "savedtf" }, { "dtf", v } }; vals[DATE_TICKER_FORMAT_VAR] = "D.M.Y";
    legacy_dtf ();
    Erg e { false, "", "", 0, leg_alert ? leg_alert : "" };
    for (auto & s : sets) if (s.first == DATE_TICKER_FORMAT_VAR) { e.gespeichert = true; e.a = s.second; }
    if (! e.gespeichert && ! leg_alert) e.code = -1;                        // abgewiesen ohne Meldung
    if (e.gespeichert && leg_alert) e.code = -2;                            // gespeichert UND Warnung
    return e;
}
static Erg api_ll (const std::string & lon, const std::string & lat, const char * city = "Aarberg")
{
    reset (); params = { { "lon", lon }, { "lat", lat } };
    vals[WEATHER_CITY_STR_VAR] = city; vals[WEATHER_LON_STR_VAR] = "7.2682"; vals[WEATHER_LAT_STR_VAR] = "47.0441";
    http_api_weather_coordinates_set ();
    Erg e { false, "", "", last_error, last_detail };
    for (auto & s : sets) { if (s.first == WEATHER_LON_STR_VAR) { e.gespeichert = true; e.a = s.second; } if (s.first == WEATHER_LAT_STR_VAR) e.b = s.second; }
    return e;
}
static Erg leg_ll (const std::string & lon, const std::string & lat, const char * city = "Aarberg")
{
    reset (); params = { { "action", "savelonlat" }, { "lon", lon }, { "lat", lat } };
    vals[WEATHER_CITY_STR_VAR] = city; vals[WEATHER_LON_STR_VAR] = "7.2682"; vals[WEATHER_LAT_STR_VAR] = "47.0441";
    legacy_lonlat ();
    Erg e { false, "", "", 0, leg_alert ? leg_alert : "" };
    for (auto & s : sets) { if (s.first == WEATHER_LON_STR_VAR) { e.gespeichert = true; e.a = s.second; } if (s.first == WEATHER_LAT_STR_VAR) e.b = s.second; }
    if (! e.gespeichert && ! leg_alert) e.code = -1;
    return e;
}

/* ---- Teil T ---- */
static int t_faelle[3][2];
static void pruefe_weg (const char * weg, const std::string & feld, const std::string & ein, bool soll, const Erg & e)
{
    std::string wie = std::string (weg) + " " + feld + " '" + ein + "'";
    if (e.code < 0) { fehl ("T", n, wie + ": Antwort widerspruechlich (" + std::to_string (e.code) + ")"); return; }
    if (e.gespeichert != soll) { fehl ("T", n, wie + (e.gespeichert ? ": angenommen, erwartet abgewiesen" : ": abgewiesen (" + e.detail + "), erwartet angenommen")); return; }
    if (soll)
    {
        std::string erw = feld == "datumsformat" ? ein : komma_zu_punkt (ein);
        std::string gesp = feld == "breite" ? e.b : e.a;
        if (gesp != erw) fehl ("T", n, wie + ": gespeichert '" + gesp + "' statt '" + erw + "'");
    }
    else if (std::string (weg).rfind ("API", 0) == 0 && ! ein.empty () && e.code != 2)
        fehl ("T", n, wie + ": Fehlerkennung " + std::to_string (e.code) + " statt 2 (HTTP_API_ERROR_OUT_OF_RANGE)");
}
static void teil_t (void)
{
    FILE * f = fopen (TABELLE, "rb"); if (! f) { printf ("KAPUTT: Tabelle fehlt\n"); exit (2); }
    char zeile[512]; bool kopf = false;
    while (fgets (zeile, sizeof zeile, f))
    {
        std::string z (zeile); if (! z.empty () && z.back () == '\n') z.pop_back ();
        if (z.empty () || z[0] == '#') continue;
        if (! kopf) { kopf = true; continue; }
        std::vector<std::string> sp; size_t a = 0, b;
        while ((b = z.find ('\t', a)) != std::string::npos) { sp.push_back (z.substr (a, b - a)); a = b + 1; }
        sp.push_back (z.substr (a));
        if (sp.size () < 3 || (sp[2] != "gueltig" && sp[2] != "ungueltig")) { printf ("KAPUTT: Tabellenzeile '%s'\n", z.c_str ()); exit (2); }
        const std::string & feld = sp[0], & ein = sp[1]; bool soll = sp[2] == "gueltig";
        n++;
        int fi = feld == "datumsformat" ? 0 : feld == "laenge" ? 1 : feld == "breite" ? 2 : -1;
        if (fi < 0) { printf ("KAPUTT: Feld '%s'\n", feld.c_str ()); exit (2); }
        t_faelle[fi][soll]++;
        if (fi == 0)
        {
            pruefe_weg ("API date_ticker_format_set", feld, ein, soll, api_dtf (ein));
            pruefe_weg ("Legacy savedtf", feld, ein, soll, leg_dtf (ein));
        }
        else
        {
            std::string lon = fi == 1 ? ein : "8.5400", lat = fi == 2 ? ein : "47.3769";
            pruefe_weg ("API weather_coordinates_set", feld, ein, soll, api_ll (lon, lat));
            pruefe_weg ("Legacy savelonlat", feld, ein, soll, leg_ll (lon, lat));
        }
    }
    fclose (f);
}

/* ---- Teil P: Paar und Leerstand ---- */
static void p_fall (const char * name, const std::string & lon, const std::string & lat, const char * city, bool soll, int code, const char * detail)
{
    n++;
    Erg a = api_ll (lon, lat, city), l = leg_ll (lon, lat, city);
    if (a.gespeichert != soll) fehl ("P", n, std::string ("API ") + name + (a.gespeichert ? ": angenommen" : ": abgewiesen"));
    else if (! soll && (a.code != code || a.detail != detail)) fehl ("P", n, std::string ("API ") + name + ": " + std::to_string (a.code) + " '" + a.detail + "'");
    if (l.gespeichert != soll) fehl ("P", n, std::string ("Legacy ") + name + (l.gespeichert ? ": angenommen" : ": abgewiesen"));
    else if (! soll && l.detail != detail) fehl ("P", n, std::string ("Legacy ") + name + ": '" + l.detail + "'");
}
static void teil_p (void)
{
    p_fall ("beide leer, Ort da", "", "", "Aarberg", true, 0, "");
    p_fall ("beide leer, kein Ort", "", "", "", false, 1, "lon and lat missing or empty and no city configured");
    p_fall ("nur lon", "8.5400", "", "Aarberg", false, 1, "lon and lat required together");
    p_fall ("nur lat", "", "47.3769", "Aarberg", false, 1, "lon and lat required together");
    p_fall ("lon ungueltig", "<\"&", "47.3769", "Aarberg", false, 2, "lon invalid (decimal number -180..180, e.g. 8.5400)");
    p_fall ("lat ungueltig", "8.5400", "<\"&", "Aarberg", false, 2, "lat invalid (decimal number -90..90, e.g. 47.3769)");
    p_fall ("beide ungueltig, lon zuerst", "x", "y", "Aarberg", false, 2, "lon invalid (decimal number -180..180, e.g. 8.5400)");
    /* API: Laengengrenze vor der Inhaltsregel, Meldung unveraendert */
    n++; Erg e = api_ll ("8.5400001", "47.3769");
    if (e.gespeichert || e.code != 2 || e.detail != "lon too long (max. 8 bytes)") fehl ("P", n, "API 9 Byte: " + std::to_string (e.code) + " '" + e.detail + "'");
    n++; e = api_dtf ("");
    if (e.gespeichert || e.code != 1 || e.detail != "value missing or empty") fehl ("P", n, "API dtf leer: " + std::to_string (e.code) + " '" + e.detail + "'");
    n++; e = api_dtf ("D.M.Y.");
    if (e.gespeichert || e.code != 2 || e.detail != "value too long (max. 5 bytes)") fehl ("P", n, "API dtf 6 Byte: " + std::to_string (e.code) + " '" + e.detail + "'");
#ifdef HAVE_REGEL
    n++; e = api_dtf ("<scri");
    if (e.gespeichert || e.code != 2 || e.detail != http_date_ticker_format_invalid) fehl ("P", n, "API dtf <scri: " + std::to_string (e.code) + " '" + e.detail + "'");
    n++; e = leg_dtf ("yyyyy");
    if (e.gespeichert || e.detail != http_date_ticker_format_invalid) fehl ("P", n, "Legacy dtf yyyyy: '" + e.detail + "'");
#endif
}

/* ---- Teil O: Orakel ---- */
#ifdef HAVE_REGEL
static long o_faelle;
static bool orakel_dtf (const std::string & s)
{
    static const std::regex form ("^[dDmMyY./ \\-]{1,5}$");
    if (! std::regex_match (s, form)) return false;
    if (s.front () == ' ' || s.back () == ' ') return false;
    int t = 0, m = 0, j = 0;
    for (char c : s) { if (c == 'd' || c == 'D') t++; if (c == 'm' || c == 'M') m++; if (c == 'y' || c == 'Y') j++; }
    return t + m + j >= 1 && t <= 1 && m <= 1 && j <= 1;
}
static bool orakel_koord (std::string s, double grenze)
{
    static const std::regex form ("^-?[0-9]+(\\.[0-9]+)?$");
    s = komma_zu_punkt (s);
    if (s.size () > 8 || ! std::regex_match (s, form)) return false;
    return fabs (strtod (s.c_str (), 0)) <= grenze;
}
static int o_fehl;
static void o_dtf (const std::string & s)
{
    o_faelle++;
    bool soll = orakel_dtf (s), ist = http_date_ticker_format_ok (s.c_str ());
    if (soll != ist && o_fehl++ < 10) fehl ("O", n, "Datumsformat '" + s + "': Funktion " + (ist ? "gueltig" : "ungueltig"));
}
static void o_koord (const std::string & s)
{
    char b[32]; snprintf (b, sizeof b, "%s", s.c_str ()); strsubst (b, ',', '.');
    for (int g : { 180, 90 })
    {
        o_faelle++;
        bool soll = orakel_koord (s, g), ist = http_coordinate_ok (b, 8, g);
        if (soll != ist && o_fehl++ < 10) fehl ("O", n, "Koordinate '" + s + "' Grenze " + std::to_string (g) + ": Funktion " + (ist ? "gueltig" : "ungueltig"));
    }
}
static void alle (const char * alpha, int maxlen, void (*f) (const std::string &))
{
    std::vector<std::string> ebene { "" }; f ("");
    for (int l = 1; l <= maxlen; l++)
    {
        std::vector<std::string> neu;
        for (auto & s : ebene) for (const char * a = alpha; *a; a++) { neu.push_back (s + *a); f (neu.back ()); }
        ebene.swap (neu);
    }
}
static void teil_o (void)
{
    long vorher;
    n++; vorher = o_faelle; alle ("dDmMyY./- x<", 5, o_dtf); long od = o_faelle - vorher;
    n++; vorher = o_faelle; alle ("-0189.,+e ", 6, o_koord);
    char b[32];
    for (long k = -181000; k <= 181000; k++)
    {
        for (const char * fmt : { "%.3f", "%.4f", "%.0f", "%.1f" })
        { snprintf (b, sizeof b, fmt, k / 1000.0); o_koord (b); }
    }
    srand (12345);
    const char * zahl = "-0123456789.,";
    for (int i = 0; i < 300000; i++)
    {
        int l = 7 + rand () % 3; std::string s;
        for (int j = 0; j < l; j++) s += zahl[rand () % 13];
        o_koord (s);
    }
    long ok = o_faelle - vorher;
    printf ("  Orakel: %ld Datumsformate, %ld Koordinatenpruefungen, %d Abweichungen\n", od, ok, o_fehl);
}
#endif

int main ()
{
    int t0 = n; teil_t (); int nt = n - t0;
    int p0 = n; teil_p (); int np = n - p0;
    printf ("  Tabelle: %d Faelle (Datumsformat %d gueltig / %d ungueltig, Laenge %d / %d, Breite %d / %d), je zwei Wege\n", nt,
            t_faelle[0][1], t_faelle[0][0], t_faelle[1][1], t_faelle[1][0], t_faelle[2][1], t_faelle[2][0]);
#ifdef HAVE_REGEL
    teil_o ();
#else
    printf ("  Orakel: AUSGELASSEN - die Prueffunktionen fehlen in diesem Stand (zaehlt nicht)\n");
#endif
    printf ("%s (%s): %d Tabellen- und %d Paarfaelle, %d Fehler\n", fails ? "FEHLGESCHLAGEN" : "OK", BEZEICHNUNG, nt, np, fails);
    return fails ? 1 : 0;
}
