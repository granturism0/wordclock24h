/* Pruefstand E.2b (Teil D, ESP-Seite, Paket 2026-10-09): der eine Kommandoabsender, AKD.1-AKD.4.
 *
 * Uebersetzt den ECHTEN vars.cpp (vollstaendig), den Tetris-Fall aus udpsrv.cpp, aus ESP-uclock.ino
 * var_crc, den Absender und var_frame_line samt Zustand, aus stm32flash.cpp stm32_reset und
 * stm32_activate_bootloader. Referenz (-DREF): dieselben Formen aus dem letzten ESP-Release vor Teil D;
 * ihr Strom wird byte-genau verglichen (AKD.2).
 *
 * Jedes Szenario laeuft in einem eigenen Kindprozess, abgezweigt VOR jeder Zustandsaenderung -- das
 * ist der Zustand nach dem ESP-Start, Weg (a).
 */
#include "Arduino.h"
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include "base.h"
#include "vars.h"
T14Serial Serial;
uint32_t t14_now = 1000;
static std::vector<std::string> logring;
void stm32_log_append (const char * l) { logring.push_back (l); }
uint16_t htoi (char * s, uint8_t n) { uint16_t v = 0; for (int i = 0; i < n && s[i]; i++) { char c = s[i]; v = (uint16_t) (v * 16 + (c >= 'a' ? c - 'a' + 10 : c >= 'A' ? c - 'A' + 10 : c - '0')); } return v; }   /* nur fuer var_set_parameter, hier ungenutzt */
int dayofweek (int, int, int) { return 0; }                                   /* dito */

/* --- Tetris-Fall aus udpsrv.cpp --- */
#define LISTENER_TETRIS_CODE 'g'
#define LISTENER_DISCOVER_CODE 'd'
static void tetris_send (const char * paket)
{
  char udp_server_packet_buffer[64]; strcpy (udp_server_packet_buffer, paket);
  switch (udp_server_packet_buffer[0]) {
#include "tetris_code.inc"
  ; }
}

#ifndef REF
/* --- aus ESP-uclock.ino und stm32flash.cpp --- */
#define STM32_LOG_LINE_LEN 120
static int digital_low_reset;
static void digitalWrite (int pin, int v) { (void) pin; (void) v; }
static void pinMode (int, int) {}
static int stm32_serial_poll (int, int) { return -1; }
#include "ino_code.inc"
#include "flash_code.inc"
#endif

#include "forms.inc"

/* Schiedsrichter: tools/checks/var-crc.c (umbenannt) */
#include "schiri.inc"

static int fails, checks;
#define PRUEF(cond, ...) do { checks++; if (! (cond)) { fails++; printf ("FEHL %s: ", szen); printf (__VA_ARGS__); printf ("\n"); } } while (0)
static const char * szen = "";

static std::string esc (const std::string & s) { std::string r; for (char c : s) { if (c == '\r') r += "\\r"; else if (c == '\n') r += "\\n"; else r += c; } return r; }
static std::vector<std::string> split (const std::string & s)
{ std::vector<std::string> v; size_t p = 0, q; while ((q = s.find ("\r\n", p)) != std::string::npos) { v.push_back (s.substr (p, q + 2 - p)); p = q + 2; } if (p < s.size ()) v.push_back (s.substr (p)); return v; }

/* Alle Formen ausloesen, Zeilen einsammeln */
static std::vector<std::string> alle_formen ()
{
  std::vector<std::string> r; form_init ();
  for (size_t i = 0; i < sizeof formen / sizeof formen[0]; i++) { Serial.out.clear (); formen[i] (); auto z = split (Serial.out);
    if ((int) z.size () != formen_zeilen[i]) { printf ("FEHL Form %zu: %zu Zeilen statt %d\n", i, z.size (), formen_zeilen[i]); fails++; }
    for (auto & x : z) r.push_back (x); }
  return r;
}

#ifdef REF
int main (int argc, char ** argv)
{
  FILE * f = fopen (argv[1], "wb"); auto z = alle_formen ();
  for (auto & x : z) { uint32_t n = x.size (); fwrite (&n, 4, 1, f); fwrite (x.data (), 1, n, f); }
  fclose (f); printf ("Referenz: %zu Zeilen aus %zu Aufrufen\n", z.size (), sizeof formen / sizeof formen[0]); return fails ? 1 : 0;
}
#else
static std::vector<std::string> ref;

static void lerne (const char * vb) { char b[16]; strcpy (b, vb); var_frame_line (b); }
static bool alle_cmd (const std::vector<std::string> & z) { for (auto & x : z) if (x.compare (0, 4, "CMD ")) return false; return true; }
static std::string zuletzt_log () { return logring.empty () ? "" : logring.back (); }

/* AKD.3: jede Zeile CMC, Nutzlast gleich der Referenz, Marke gegen den Schiedsrichter */
static void pruef_markiert (const std::vector<std::string> & z)
{
  PRUEF (z.size () == ref.size (), "%zu Zeilen, Referenz %zu", z.size (), ref.size ());
  for (size_t i = 0; i < z.size () && i < ref.size (); i++) {
    std::string nutz = ref[i].substr (4, ref[i].size () - 6);
    char m[8]; snprintf (m, sizeof m, "*%04x", schiri_crc (nutz.c_str ()));
    std::string soll = "CMC " + nutz + m + "\r\n";
    PRUEF (z[i] == soll, "Form %zu: '%.70s' statt '%.70s'", i, esc (z[i]).c_str (), esc (soll).c_str ());
  }
}

static int szenario (const char * name, void (* fn) ())
{
  fflush (stdout); pid_t p = fork ();
  if (p == 0) { szen = name; fails = checks = 0; fn (); printf ("  %-58s %d Pruefungen, %d Fehler\n", name, checks, fails); fflush (stdout); _exit (fails ? 1 : 0); }
  int st; waitpid (p, &st, 0); return WIFEXITED (st) ? WEXITSTATUS (st) : 1;
}

int main (int argc, char ** argv)
{
  FILE * f = fopen (argv[1], "rb"); uint32_t n;
  while (fread (&n, 4, 1, f) == 1) { std::string s (n, '\0'); fread (&s[0], 1, n, f); ref.push_back (s); } fclose (f);
  int fehl = 0;

  fehl += szenario ("(a) ESP-Start: jede Form byte-gleich (AKD.2)", [] {
    auto z = alle_formen ();
    PRUEF (z.size () == ref.size (), "%zu Zeilen, Referenz %zu", z.size (), ref.size ());
    for (size_t i = 0; i < z.size () && i < ref.size (); i++) PRUEF (z[i] == ref[i], "Form %zu: '%.70s' statt '%.70s'", i, esc (z[i]).c_str (), esc (ref[i]).c_str ());
    PRUEF (logring.empty (), "%zu Logzeilen ohne Wechsel", logring.size ()); });

  fehl += szenario ("Eroeffnung mit 0x04: jede Form als CMC (AKD.3)", [] {
    lerne ("VB0701");
    PRUEF (logring.size () == 1 && zuletzt_log ().find ("Faehigkeit an") != std::string::npos, "Logring '%s' (%zu)", zuletzt_log ().c_str (), logring.size ());
    pruef_markiert (alle_formen ()); });

  fehl += szenario ("Schiedsrichter-Vektoren aus var-crc.c (AKD.3)", [] {
    lerne ("VB0401");
    static const char * const v[] = { "OT0002", "OT000", "OT0", "N0a0f", "S03meinhost", "", "DC00ff800000ff", "T0120261004213000" };
    for (auto x : v) { Serial.out.clear ();
#ifndef OHNE_ABSENDER
      stm_cmd_send (x);
#endif
      char m[8]; snprintf (m, sizeof m, "*%04x", schiri_crc (x));
      PRUEF (Serial.out == std::string ("CMC ") + x + m + "\r\n", "'%s' => '%s'", x, esc (Serial.out).c_str ()); } });

  fehl += szenario ("Eroeffnung ohne 0x04 (0x03): bleibt CMD", [] {
    lerne ("VB0301"); PRUEF (alle_cmd (alle_formen ()), "markiert ohne 0x04"); PRUEF (logring.empty (), "Logzeile ohne Wechsel"); });

  fehl += szenario ("verlorene Eroeffnung (nur VE): bleibt CMD", [] {
    lerne ("VE01"); PRUEF (alle_cmd (alle_formen ()), "markiert ohne Eroeffnung"); });

  fehl += szenario ("nachgesendete Eroeffnung desselben Abgleichs aendert nichts", [] {
    lerne ("VB0705"); lerne ("VE05"); size_t l = logring.size (); lerne ("VB0705");
    PRUEF (logring.size () == l, "nachgesendete Eroeffnung erzeugte Logzeile");
    pruef_markiert (alle_formen ()); });

  fehl += szenario ("zweite Eroeffnung mit 0x04: kein zweiter Wechsel", [] {
    lerne ("VB0701"); lerne ("VE01"); lerne ("VB0702");
    int an = 0; for (auto & l : logring) an += l.find ("Faehigkeit an") != std::string::npos;
    PRUEF (an == 1, "%d Zeilen 'Faehigkeit an'", an); });

  fehl += szenario ("(b) Eroeffnung ohne 0x04 loescht", [] {
    lerne ("VB0701"); lerne ("VE01"); lerne ("VB0302");
    PRUEF (zuletzt_log ().find ("Faehigkeit aus (Eroeffnung)") != std::string::npos, "Logring '%s'", zuletzt_log ().c_str ());
    PRUEF (alle_cmd (alle_formen ()), "markiert nach Eroeffnung ohne 0x04"); });

  fehl += szenario ("(c) STM-Reset loescht", [] {
    lerne ("VB0701");
#ifndef OHNE_ABSENDER
    stm32_reset ();
#endif
    PRUEF (zuletzt_log ().find ("Faehigkeit aus (STM-Reset)") != std::string::npos, "Logring '%s'", zuletzt_log ().c_str ());
    PRUEF (alle_cmd (alle_formen ()), "markiert nach STM-Reset"); });

  fehl += szenario ("(d) STM-Flash loescht", [] {
    lerne ("VB0701");
#ifndef OHNE_ABSENDER
    stm32_activate_bootloader ();
#endif
    PRUEF (zuletzt_log ().find ("Faehigkeit aus (STM-Flash)") != std::string::npos, "Logring '%s'", zuletzt_log ().c_str ());
    PRUEF (alle_cmd (alle_formen ()), "markiert nach STM-Flash"); });

  fehl += szenario ("(c) dann neue Eroeffnung mit 0x04: wieder CMC", [] {
    lerne ("VB0701"); lerne ("VE01");
#ifndef OHNE_ABSENDER
    stm32_reset ();
#endif
    lerne ("VB0702"); pruef_markiert (alle_formen ()); });

  fehl += szenario ("keine Werte im Logring", [] {
    lerne ("VB0701"); alle_formen (); lerne ("VB0302");
    for (auto & l : logring) PRUEF (l.find ("Hallo") == std::string::npos && l.find ("Gute") == std::string::npos && l.size () < 48, "'%s'", l.c_str ()); });

  printf ("%s (%s): %zu Kommandozeilen aus %d Absendestellen, %d Szenarien fehlgeschlagen\n",
          fehl ? "FEHLGESCHLAGEN" : "OK", BEZEICHNUNG, ref.size (), FORM_STELLEN, fehl);
  return fehl ? 1 : 0;
}
#endif
