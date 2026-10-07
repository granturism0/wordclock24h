/* Pruefstand S.5 (N1, AKS.14) samt Ergaenzung des Leads: n_overlays > MAX_OVERLAYS vom STM.
 *
 * Uebersetzt den ECHTEN Code aus http.cpp (extract.py): http_overlays() vollstaendig, die
 * Hilfsfunktion http_n_overlays_for_read() (nur neu), http_get_overlay_idx_param(),
 * http_api_overlay_display(), http_api_overlay_delete(). Alle HTML-Ausgaben sind Attrappen.
 *
 * ZIELTYPEN (L256): uint_fast8_t ist auf dem ESP unsigned int, 32 Bit
 * (xtensa-lx106-elf-gcc: __UINT_FAST8_TYPE__ unsigned int), auf dem Mac unsigned char. Gerade
 * daran haengt N1: "oid256" waere auf dem Mac 0 und harmlos, auf dem ESP overlays[256].
 * Deshalb wird uint_fast8_t hier auf unsigned int abgebildet; int ist auf beiden 32 Bit.
 *
 * SPEICHERWAECHTER: overlays zeigt auf ein groesseres Feld; was hinter Platz 31 landet, faellt
 * am Waechtermuster auf. Ein Index weit ausserhalb stuerzt ab -- jeder Fall laeuft deshalb in
 * einem eigenen Prozess, ein Absturz ist ein Fehlschlag.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <map>
#ifndef NATIV
#define uint_fast8_t  unsigned int
#define uint_fast16_t unsigned int
#endif
#define overlays overlays_ptr
#include VARSHDR
#undef overlays
#define OVERLAY_HEADER_COLS 10
#define HTTP_API_ERROR_MISSING_VALUE 1
#define HTTP_API_ERROR_OUT_OF_RANGE  2

#define GUARD 300
static OVERLAY  overlay_mem[MAX_OVERLAYS + GUARD];
#define overlays overlay_mem

unsigned int numvars[MAX_NUM_VARIABLES];
static int   sent_display = -1;
unsigned int get_numvar (NUM_VARIABLE v) { return v < MAX_NUM_VARIABLES ? numvars[v] : 0; }
unsigned int set_numvar (NUM_VARIABLE v, unsigned int x)
{
    if (v < MAX_NUM_VARIABLES) numvars[v] = x & 0xFFFF;         /* wie die Bruecke: 16 Bit */
    if (v == DISPLAY_OVERLAY_NUM_VAR) sent_display = (int) (x & 0xFFFF);
    return 1;
}
static int max_set_overlay_var = -1;
unsigned int set_overlay_var (uint_fast8_t i) { if ((int) i > max_set_overlay_var) max_set_overlay_var = (int) i; return 1; }
void utf8_copy_truncated (char * d, const char * s, unsigned int n) { strncpy (d, s, n); d[n] = 0; }

static std::map<std::string, std::string> params;
static char parambuf[16][64]; static int parami;
static char * http_get_param (const char * n)
{
    char * b = parambuf[parami++ % 16]; snprintf (b, 64, "%s", params.count (n) ? params[n].c_str () : ""); return b;
}
static int  http_get_checkbox_param (const char * n) { return params.count (n) ? 1 : 0; }
static int  http_get_int_param (const char * n, int * v) { if (! params.count (n)) return 0; *v = atoi (params[n].c_str ()); return 1; }
static int  last_error = -1;
static void http_json_error (int code, const char *) { last_error = code; }
static void http_json_ok (void) { last_error = 0; }

/* HTML-Attrappen. save_column() merkt sich die hoechste gerenderte ZEILE der Liste. */
static int in_row = 0, max_row = -1;
template<typename... T> static void nop (T...) {}
#define http_header(...)                nop (__VA_ARGS__)
#define begin_box(...)                  nop (__VA_ARGS__)
#define end_box(...)                    nop (__VA_ARGS__)
#define message_icon_files_missing()    nop ()
#define message_tables_file_missing()   nop ()
#define table_header(...)               nop (__VA_ARGS__)
#define table_trailer()                 nop ()
#define checkbox_column(...)            nop (__VA_ARGS__)
#define select_column(...)              nop (__VA_ARGS__)
#define select_icons(...)               nop (__VA_ARGS__)
#define begin_column()                  nop ()
#define end_column()                    nop ()
#define input_column(...)               nop (__VA_ARGS__)
#define input_field(...)                nop (__VA_ARGS__)
#define text_column(...)                nop (__VA_ARGS__)
#define begin_form(...)                 nop (__VA_ARGS__)
#define end_form()                      nop ()
#define http_send_FS(...)               nop (__VA_ARGS__)
#define http_send(...)                  nop (__VA_ARGS__)
#define http_trailer()                  nop ()
#define http_flush()                    nop ()
static void begin_table_row_form (const char *) { in_row = 1; }
static void end_table_row_form (void) { in_row = 0; }
static void save_column (const char * id) { int k = atoi (id + 3); if (in_row && k > max_row) max_row = k; }

#include "n1_code.inc"

/* ---------------------------------------------------------------- Hilfen */
static int fails;
static void expect (bool ok, const char * what, const std::string & got = "")
{
    if (! ok) { fails++; printf ("      FEHLER: %s%s%s\n", what, got.empty () ? "" : " -- bekam: ", got.c_str ()); }
}
static void fill (unsigned n)
{
    memset (overlay_mem, 0xA5, sizeof overlay_mem);
    for (int i = 0; i < MAX_OVERLAYS; i++) { memset (&overlay_mem[i], 0, sizeof (OVERLAY)); overlay_mem[i].type = 1; overlay_mem[i].interval = (unsigned) i; }
    numvars[OVERLAY_N_OVERLAYS_NUM_VAR] = n;
}
static bool guard_intact (void)
{
    const unsigned char * p = (const unsigned char *) &overlay_mem[MAX_OVERLAYS];
    for (size_t i = 0; i < GUARD * sizeof (OVERLAY); i++) if (p[i] != 0xA5) return false;
    return true;
}
static void legacy (const char * action, unsigned n)
{
    fill (n); params.clear (); params["action"] = action; params["otype"] = "6"; params["oint"] = "7"; params["oname"] = "X";
    http_overlays ();
}
static std::string num (long v) { return std::to_string (v); }

/* ---------------------------------------------------------------- Faelle */
static void f_anhaengen (void)   { legacy ("saveoid3", 3); expect (numvars[OVERLAY_N_OVERLAYS_NUM_VAR] == 4 && overlay_mem[3].type == 6, "oid == n haengt an (n 3 -> 4)", num (numvars[OVERLAY_N_OVERLAYS_NUM_VAR])); expect (guard_intact (), "Waechter unversehrt"); }
static void f_aendern (void)     { legacy ("saveoid2", 3); expect (numvars[OVERLAY_N_OVERLAYS_NUM_VAR] == 3 && overlay_mem[2].type == 6, "oid < n aendert, n bleibt 3"); }
static void f_luecke (void)      { legacy ("saveoid5", 3); expect (overlay_mem[5].type == 1 && numvars[OVERLAY_N_OVERLAYS_NUM_VAR] == 3 && max_set_overlay_var < 0, "oid 5 bei n 3 (Luecke) => abgewiesen, nichts geschrieben"); }
static void f_voll (void)        { legacy ("saveoid32", 32); expect (guard_intact () && numvars[OVERLAY_N_OVERLAYS_NUM_VAR] == 32 && max_set_overlay_var < 0, "oid 32 bei n 32 => abgewiesen, n bleibt 32", num (numvars[OVERLAY_N_OVERLAYS_NUM_VAR])); }
static void f_40 (void)          { legacy ("oid40", 3);     expect (guard_intact () && max_set_overlay_var < 0, "oid 40 (per <img>) => abgewiesen"); }
static void f_256 (void)         { legacy ("oid256", 3);    expect (guard_intact () && max_set_overlay_var < 0, "oid 256 (auf dem Mac waere das 0) => abgewiesen"); }
static void f_neg (void)         { legacy ("saveoid-1", 3); expect (guard_intact () && max_set_overlay_var < 0, "oid -1 => abgewiesen"); }
static void f_sentinel (void)    { legacy ("oid255", 3);    expect (max_set_overlay_var < 0 && numvars[OVERLAY_N_OVERLAYS_NUM_VAR] == 3, "oid 255 (Merkwert) => nichts"); }
static void f_disp_ok (void)     { legacy ("disp2", 3);     expect (sent_display == 2, "disp2 bei n 3 => an den STM", num (sent_display)); }
static void f_disp_40 (void)     { legacy ("disp40", 3);    expect (sent_display == -1, "disp40 => nicht an den STM", num (sent_display)); }
static void f_disp_neg (void)    { legacy ("disp-1", 3);    expect (sent_display == -1, "disp-1 => nicht an den STM", num (sent_display)); }
static void f_disp_n (void)      { legacy ("disp3", 3);     expect (sent_display == -1, "disp3 bei n 3 (unbelegt) => nicht an den STM", num (sent_display)); }
static void liste (unsigned n)   { legacy ("", n); expect (max_row == (int) (n < MAX_OVERLAYS ? n : MAX_OVERLAYS) - 1, "Liste liest hoechstens bis Platz 31", "hoechste Zeile " + num (max_row)); }
static void f_liste32 (void)     { liste (32); }
static void f_liste33 (void)     { liste (33); }
static void f_liste255 (void)    { liste (255); }
static void f_save_n33 (void)    { legacy ("saveoid33", 33); expect (guard_intact () && max_set_overlay_var < 0, "oid 33 bei gemeldetem n 33 => abgewiesen"); }
static void api_del (int idx, unsigned n) { fill (n); params.clear (); params["idx"] = num (idx); http_api_overlay_delete (); }
static void f_del_ok (void)      { api_del (1, 3); expect (last_error == 0 && numvars[OVERLAY_N_OVERLAYS_NUM_VAR] == 2 && overlay_mem[1].interval == 2 && guard_intact (), "loeschen idx 1 bei n 3 => rueckt nach, n 2"); }
static void f_del_32 (void)      { api_del (31, 32); expect (last_error == 0 && numvars[OVERLAY_N_OVERLAYS_NUM_VAR] == 31 && guard_intact (), "loeschen idx 31 bei n 32 => erlaubt, Waechter unversehrt"); }
static void f_del_33 (void)      { api_del (0, 33); expect (guard_intact () && numvars[OVERLAY_N_OVERLAYS_NUM_VAR] == 33 && last_error == HTTP_API_ERROR_OUT_OF_RANGE, "loeschen bei gemeldetem n 33 => abgewiesen, nichts geschrieben", "Fehler " + num (last_error)); }
static void f_del_255 (void)     { api_del (0, 255); expect (guard_intact () && numvars[OVERLAY_N_OVERLAYS_NUM_VAR] == 255 && last_error == HTTP_API_ERROR_OUT_OF_RANGE, "loeschen bei gemeldetem n 255 => abgewiesen", "Fehler " + num (last_error)); }
static void f_disp_api_33 (void) { fill (33); params.clear (); params["idx"] = "32"; http_api_overlay_display (); expect (sent_display == -1 && last_error == HTTP_API_ERROR_OUT_OF_RANGE, "overlay_display idx 32 bei n 33 => abgewiesen", num (sent_display)); }

struct Fall { const char * name; void (*fn) (void); };
static Fall faelle[] = {
    { "Legacy: anhaengen oid == n",                 f_anhaengen },
    { "Legacy: aendern oid < n",                    f_aendern },
    { "Legacy: Luecke oid > n",                     f_luecke },
    { "Legacy: oid 32 bei n 32",                    f_voll },
    { "Legacy: oid 40",                             f_40 },
    { "Legacy: oid 256 (Typbreite)",                f_256 },
    { "Legacy: oid -1",                             f_neg },
    { "Legacy: oid 255 = kein Auftrag",             f_sentinel },
    { "Legacy: disp gueltig",                       f_disp_ok },
    { "Legacy: disp 40",                            f_disp_40 },
    { "Legacy: disp -1",                            f_disp_neg },
    { "Legacy: disp == n",                          f_disp_n },
    { "Legacy-Liste n = 32",                        f_liste32 },
    { "Legacy-Liste n = 33 (vom STM)",              f_liste33 },
    { "Legacy-Liste n = 255 (vom STM)",             f_liste255 },
    { "Legacy: oid 33 bei n = 33",                  f_save_n33 },
    { "API loeschen, n = 3",                        f_del_ok },
    { "API loeschen, n = 32",                       f_del_32 },
    { "API loeschen, n = 33 (vom STM)",             f_del_33 },
    { "API loeschen, n = 255 (vom STM)",            f_del_255 },
    { "API anzeigen idx 32, n = 33",                f_disp_api_33 },
};
int main (void)
{
    int n = sizeof faelle / sizeof faelle[0], bad = 0;
    printf ("Pruefstand S.5 / N1 (%s), %d Faelle, uint_fast8_t = %zu Byte\n", BEZEICHNUNG, n, sizeof (uint_fast8_t));
    for (int i = 0; i < n; i++)
    {
        fflush (stdout);
        pid_t pid = fork ();
        if (pid == 0) { faelle[i].fn (); fflush (stdout); _exit (fails ? 1 : 0); }
        int st; waitpid (pid, &st, 0);
        int ok = WIFEXITED (st) && WEXITSTATUS (st) == 0;
        printf ("  [%s] %s%s\n", ok ? " ok " : "FEHL", faelle[i].name, WIFSIGNALED (st) ? " (ABGESTUERZT)" : "");
        bad += ! ok;
    }
    printf ("Ergebnis: %d von %d Faellen bestanden\n", n - bad, n);
    return bad ? 1 : 0;
}
