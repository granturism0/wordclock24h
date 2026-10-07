/* Pruefstand S.4 (C31, AKS.10): eeprom_read() schreibt die beiden Schluessel nicht mehr im Klartext.
 * Geprueft an der FORM der Zeile: "EEPROM ssidkey: " bzw. "EEPROM AP ssidkey: " gefolgt von genau
 * "gesetzt" oder "leer" -- und der Schluessel selbst kommt in der gesamten Ausgabe nicht vor.
 * Uebersetzt die echte Funktion aus eepromdata.cpp (extract.py).
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#define EEPROM_MAGIC_OFFSET 0
#define EEPROM_MAGIC_LEN 8
#define EEPROM_VERSION_OFFSET 1
#define EEPROM_VERSION_LEN 8
#define EEPROM_SSID_OFFSET 2
#define EEPROM_SSID_LEN 32
#define EEPROM_SSID_KEY_OFFSET 3
#define EEPROM_SSID_KEY_LEN 64
#define EEPROM_AP_SSID_OFFSET 4
#define EEPROM_AP_SSID_LEN 32
#define EEPROM_AP_SSID_KEY_OFFSET 5
#define EEPROM_AP_SSID_KEY_LEN 64
#define EEPROM_FLAGS_OFFSET 6
#define EEPROM_MAGIC_CONTENT "MAGIC"
struct SerialStub {
    std::string out;
    void print (const char * s) { out += s; }
    void println (const char * s) { out += s; out += "\r\n"; }
    void println (int v) { out += std::to_string (v); out += "\r\n"; }
} Serial;
char eeprom_magic[16], eeprom_version[16], eeprom_ssid[33], eeprom_ssidkey[65], eeprom_ap_ssid[33], eeprom_ap_ssidkey[65];
int  eeprom_flags;
static const char * fake[7];
static void eeprom_read_entry (char * dst, int off, int len) { strncpy (dst, fake[off], len); dst[len] = 0; }
static int  eeprom_read_byte (int) { return 3; }
static void format_eeprom (void) {}
#include "eeprom_code.inc"

static int fails;
static void fall (const char * key, const char * apkey)
{
    fake[0] = "MAGIC"; fake[1] = "110"; fake[2] = "MeinNetz"; fake[3] = key; fake[4] = "wordclock"; fake[5] = apkey;
    Serial.out.clear (); eeprom_read ();
    const std::string & o = Serial.out;
    const char * want_k  = key[0]   ? "EEPROM ssidkey: gesetzt\r\n"    : "EEPROM ssidkey: leer\r\n";
    const char * want_ap = apkey[0] ? "EEPROM AP ssidkey: gesetzt\r\n" : "EEPROM AP ssidkey: leer\r\n";
    int ok = 1;
    if (o.find (want_k)  == std::string::npos) { printf ("      FEHLER: Zeile \"%.*s\" fehlt\n", (int) strlen (want_k) - 2, want_k); ok = 0; }
    if (o.find (want_ap) == std::string::npos) { printf ("      FEHLER: Zeile \"%.*s\" fehlt\n", (int) strlen (want_ap) - 2, want_ap); ok = 0; }
    if (key[0]   && o.find (key)   != std::string::npos) { printf ("      FEHLER: WLAN-Schluessel steht in der Ausgabe\n"); ok = 0; }
    if (apkey[0] && o.find (apkey) != std::string::npos) { printf ("      FEHLER: AP-Schluessel steht in der Ausgabe\n"); ok = 0; }
    if (o.find ("EEPROM ssid: MeinNetz") == std::string::npos) { printf ("      FEHLER: SSID-Zeile veraendert\n"); ok = 0; }
    printf ("  [%s] Schluessel %s, AP-Schluessel %s\n", ok ? " ok " : "FEHL", key[0] ? "gesetzt" : "leer", apkey[0] ? "gesetzt" : "leer");
    fails += ! ok;
}
int main (void)
{
    printf ("Pruefstand S.4 / C31 (%s), 4 Faelle\n", BEZEICHNUNG);
    fall ("Geheim-4711-xyz", "1234567890");
    fall ("", "1234567890");
    fall ("Geheim-4711-xyz", "");
    fall ("", "");
    printf ("Ergebnis: %d von 4 Faellen bestanden\n", 4 - fails);
    return fails ? 1 : 0;
}
