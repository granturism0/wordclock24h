/* Pruefstand F.5g (C38 / L323, AKF.6): SSID und WLAN-Schluessel abweisen statt still kuerzen.
 *
 * Uebersetzt den ECHTEN Code aus http.cpp (extract.py): http_strvar_len_ok(), http_check_strvar_len(),
 * http_api_network_client_set(), http_api_eeprom_settings_set(), http_api_network_ap_set() und
 * (Review F.6, A4) die Zweige "savewlanlist" und "savewlan" aus http_network(). Arduino-String, WLAN und
 * EEPROM sind Attrappen; die Attrappe von String::toCharArray() kuerzt genau wie das Original
 * (hoechstens bufsize - 1 Zeichen plus Nullbyte) -- sonst saehe der Pruefstand die alte,
 * kuerzende Fassung gar nicht.
 *
 * Die Grenze ist EEPROM_*_LEN - 1 (31 bzw. 63 Byte): Mehr hat toCharArray () nie uebernommen.
 * Gezaehlt werden BYTE, nicht Zeichen (dieselbe Regel wie C22) -- 16 x "ae" sind 32 Byte.
 *
 * Typbreiten (L256): Verglichen wird strlen () gegen unsigned int; die Grenzwerte liegen weit
 * unter 2^32, size_t 32 oder 64 Bit aendert am Ergebnis nichts.
 *
 * Gegenprobe (DIR-014): Gegen ESP 3.2.25 schlagen die Faelle mit zu langen Werten an, weil die
 * alte Fassung kuerzt und {"ok":true} meldet.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <map>
#include EEPROMHDR
#define FS(x) x
#define HTTP_API_ERROR_OUT_OF_RANGE 2
#define HTTP_API_ERROR_TOO_SHORT    3

class String {
    std::string s;
public:
    String (const char * p) : s (p ? p : "") {}
    bool equals (const char * p) const { return s == p; }
    unsigned length () const { return (unsigned) s.size (); }
    void toCharArray (char * buf, unsigned size) const { unsigned n = s.size () < size - 1 ? (unsigned) s.size () : size - 1; memcpy (buf, s.data (), n); buf[n] = 0; }
};

char    eeprom_ssid[EEPROM_SSID_LEN + 1], eeprom_ssidkey[EEPROM_SSID_KEY_LEN + 1];
char    eeprom_ap_ssid[EEPROM_AP_SSID_LEN + 1], eeprom_ap_ssidkey[EEPROM_AP_SSID_KEY_LEN + 1];
uint8_t eeprom_flags;
static int saves, connects;
void eeprom_save_ssid (void) { saves++; }
void eeprom_save_ssidkey (void) { saves++; }
void eeprom_save_ap_ssid (void) { saves++; }
void eeprom_save_ap_ssidkey (void) { saves++; }
void eeprom_save_flags (void) {}
void eeprom_commit (void) {}
static void wifi_connect (const char *, const char *, bool) { connects++; }
static void wifi_ap (const char *, const char *) { connects++; }

static std::map<std::string, std::string> params;
static char * http_get_param (const char * n)
{
    static char b[8][300]; static int i; char * p = b[i++ % 8];
    snprintf (p, sizeof b[0], "%s", params.count (n) ? params[n].c_str () : ""); return p;
}
static int  last_error; static std::string last_detail; static int oks;
static void http_json_error (unsigned code, const char * d) { last_error = (int) code; last_detail = d; }
static void http_json_ok (void) { oks++; }
static void http_send (const char * s) { if (strstr (s, "\"ok\":true")) oks++; }
static void http_flush (void) {}
static void http_header (const char *, const char *, const char *) {}
static void http_trailer (void) {}
#define http_send_FS(x) http_send (x)

#include "c38_code.inc"

/* Legacy-Seite: der ausgeschnittene Block steht in einer Funktion mit den lokalen Variablen aus
 * http_network (). Rueckgabe 0 heisst "Zweig hat gespeichert und die Seite selbst beendet",
 * 1 heisst "weiter zur normalen Seite" - dort steht dann alert_message. */
static const char * legacy_alert;
static int http_network_wlan (const char * act)
{
    char *          action = (char *) act;
    const char *    alert_message = (const char *) 0;
    char            alert_buf[80];
    legacy_alert = 0;
    if (0) {}
    else
#include "c38_legacy.inc"
    legacy_alert = alert_message;
    return 1;
}

static int fails, n;
static std::string rep (const char * s, int k) { std::string r; while (k-- > 0) r += s; return r; }
static void reset (void)
{
    strcpy (eeprom_ssid, "AltNetz"); strcpy (eeprom_ssidkey, "altschluessel1");
    strcpy (eeprom_ap_ssid, "wordclock"); strcpy (eeprom_ap_ssidkey, "1234567890");
    saves = connects = oks = 0; last_error = 0; last_detail = "";
}
static bool unveraendert (void)
{
    return ! strcmp (eeprom_ssid, "AltNetz") && ! strcmp (eeprom_ssidkey, "altschluessel1")
        && ! strcmp (eeprom_ap_ssid, "wordclock") && ! strcmp (eeprom_ap_ssidkey, "1234567890") && saves == 0;
}
static void pruef (bool ok, const char * name)
{
    n++;
    printf ("  [%s] %s\n", ok ? " ok " : "FEHL", name);
    if (! ok) { fails++; printf ("        err=%d detail='%s' ok=%d saves=%d connects=%d ssid=%zu key=%zu ap_ssid=%zu ap_key=%zu Byte\n",
                                  last_error, last_detail.c_str (), oks, saves, connects, strlen (eeprom_ssid), strlen (eeprom_ssidkey), strlen (eeprom_ap_ssid), strlen (eeprom_ap_ssidkey)); }
}
static void client (const std::string & ssid, const std::string & key) { reset (); params = { {"ssid", ssid}, {"key", key} }; http_api_network_client_set (); }
static void eeset (const std::string & s, const std::string & k, const std::string & as, const std::string & ak)
{ reset (); params = { {"ssid", s}, {"key", k}, {"ap_ssid", as}, {"ap_key", ak} }; http_api_eeprom_settings_set (); }
static bool abgewiesen (void) { return last_error == HTTP_API_ERROR_OUT_OF_RANGE && oks == 0 && connects == 0 && unveraendert (); }

int main (void)
{
    std::string s31 = rep ("s", 31), s32 = rep ("s", 32), k63 = rep ("k", 63), k64 = rep ("k", 64);
    std::string ae16 = rep ("\xc3\xa4", 16), ae15x = rep ("\xc3\xa4", 15) + "x";
    printf ("Pruefstand F.5g / C38 (%s)\n", BEZEICHNUNG);

    client (s31, k63);  pruef (last_error == 0 && oks == 1 && connects == 1 && s31 == eeprom_ssid && k63 == eeprom_ssidkey, "client_set: SSID 31, Schluessel 63 Byte (Grenzwert) => angenommen, ungekuerzt");
    client (s32, "x");  pruef (abgewiesen () && last_detail.find ("ssid") == 0, "client_set: SSID 32 Byte => abgewiesen (2), kein wifi_connect, nichts gespeichert");
    client ("N", k64);  pruef (abgewiesen () && last_detail.find ("key") == 0, "client_set: Schluessel 64 Byte => abgewiesen (2), nichts gespeichert");
    client (ae16, "x"); pruef (abgewiesen (), "client_set: SSID 16 x ae = 32 Byte, 16 Zeichen => abgewiesen (Byte, nicht Zeichen)");
    client (ae15x, "x"); pruef (last_error == 0 && ae15x == eeprom_ssid, "client_set: SSID 15 x ae + x = 31 Byte => angenommen");

    eeset (s31, k63, s31, k63); pruef (last_error == 0 && oks == 1 && s31 == eeprom_ssid && k63 == eeprom_ssidkey && s31 == eeprom_ap_ssid && k63 == eeprom_ap_ssidkey, "eeprom_settings_set: alle vier am Grenzwert => angenommen, ungekuerzt");
    eeset (s32, "", "", "");    pruef (abgewiesen (), "eeprom_settings_set: SSID 32 Byte => abgewiesen, nichts gespeichert");
    eeset ("", k64, "", "");    pruef (abgewiesen (), "eeprom_settings_set: Schluessel 64 Byte => abgewiesen");
    eeset ("", "", s32, "");    pruef (abgewiesen (), "eeprom_settings_set: AP-SSID 32 Byte => abgewiesen");
    eeset (s31, k63, s31, k64); pruef (abgewiesen (), "eeprom_settings_set: nur AP-Schluessel zu lang => KEIN halber Satz (auch gueltige SSID nicht gespeichert)");
    eeset ("", "", "", "");     pruef (last_error == 0 && oks == 1 && unveraendert (), "eeprom_settings_set: alles leer => nichts geaendert (bestehende Regel)");

    /* Review F.6, A4: /api/network_ap_set */
    auto apset = [] (const std::string & s, const std::string & k) { reset (); params = { {"ssid", s}, {"key", k} }; http_api_network_ap_set (); };
    apset (s31, k63);  pruef (last_error == 0 && oks == 1 && connects == 1 && s31 == eeprom_ap_ssid && k63 == eeprom_ap_ssidkey, "network_ap_set: AP-SSID 31, Schluessel 63 Byte => angenommen, ungekuerzt");
    apset (s32, rep ("k", 10)); pruef (abgewiesen (), "network_ap_set: AP-SSID 32 Byte => abgewiesen (2), kein wifi_ap, nichts gespeichert");
    apset ("AP", k64); pruef (abgewiesen (), "network_ap_set: AP-Schluessel 64 Byte => abgewiesen (2)");
    apset ("AP", "kurz"); pruef (last_error == HTTP_API_ERROR_TOO_SHORT && unveraendert () && connects == 0, "network_ap_set: Schluessel unter 10 Zeichen => weiterhin Kennung 3");

    /* Review F.6, A4: Legacy-WLAN-Seite (http_network ()) */
    auto leg = [] (const char * act, std::map<std::string, std::string> p) { reset (); params = p; return http_network_wlan (act); };
    int r;
    r = leg ("savewlanlist", { {"ssidlist", s31}, {"keylist", k63} });
    pruef (r == 0 && connects == 1 && s31 == eeprom_ssid && k63 == eeprom_ssidkey, "Legacy savewlanlist: 31 / 63 Byte => gespeichert, ungekuerzt");
    r = leg ("savewlanlist", { {"ssidlist", s32}, {"keylist", "x"} });
    pruef (r == 1 && legacy_alert && connects == 0 && unveraendert (), "Legacy savewlanlist: SSID 32 Byte => Hinweis, kein wifi_connect, nichts gespeichert");
    r = leg ("savewlanlist", { {"ssidlist", "N"}, {"keylist", k64} });
    pruef (r == 1 && legacy_alert && connects == 0 && unveraendert (), "Legacy savewlanlist: Schluessel 64 Byte => Hinweis, nichts gespeichert");
    r = leg ("savewlanlist", { {"ssidlist", ae16}, {"keylist", "x"} });
    pruef (r == 1 && legacy_alert && unveraendert (), "Legacy savewlanlist: SSID 16 x ae = 32 Byte => Hinweis (Byte, nicht Zeichen)");
    r = leg ("savewlan", { {"ssid", s31}, {"key", k63} });
    pruef (r == 0 && connects == 1 && s31 == eeprom_ap_ssid && k63 == eeprom_ap_ssidkey, "Legacy savewlan: AP 31 / 63 Byte => gespeichert, ungekuerzt");
    r = leg ("savewlan", { {"ssid", s32}, {"key", rep ("k", 10)} });
    pruef (r == 1 && legacy_alert && connects == 0 && unveraendert (), "Legacy savewlan: AP-SSID 32 Byte => Hinweis, kein wifi_ap, nichts gespeichert");
    r = leg ("savewlan", { {"ssid", "AP"}, {"key", k64} });
    pruef (r == 1 && legacy_alert && connects == 0 && unveraendert (), "Legacy savewlan: AP-Schluessel 64 Byte => Hinweis, nichts gespeichert");
    r = leg ("savewlan", { {"ssid", "AP"}, {"key", "kurz"} });
    pruef (r == 1 && legacy_alert && ! strcmp (legacy_alert, "Minimum length of key is 10!") && unveraendert (), "Legacy savewlan: Schluessel unter 10 Zeichen => bisheriger Hinweis");

    printf ("Ergebnis: %d von %d Faellen bestanden\n", n - fails, n);
    return fails ? 1 : 0;
}
