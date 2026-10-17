/* c80i, ESP_HEAP_MAP 0 (AKI.7, Fall 8): die echte loop () und esp_heap_log () mit heapkarte.h auf 0. Erwartet: keine
 * Zeile des Instruments, kein Messpunkt (getFreeHeap nur aus der Heap-Zeile), und die Heap-Zeile unveraendert.
 * Exit 0 OK, 1 FEHL.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#define PROGMEM
#define PSTR(s) (s)
#define snprintf_P snprintf
static uint32_t sim_ms = 8000;
static uint32_t millis (void) { return sim_ms; }
static int getfree = 0;
struct { uint32_t getFreeHeap (void) { getfree++; return 7840; } uint32_t getMaxFreeBlockSize (void) { return 7104; } } ESP;
static std::vector<std::string> zeilen;
struct { bool hasOverrun (void) { return false; } size_t println (const char * s) { zeilen.push_back (s); return 0; } void flush (void) { } } Serial;
static void stm32_log_append (const char *) { }
static uint32_t esp_uart_overruns = 0;
#define ESP_HEAP_LOG                1
#define ESP_HEAP_LOG_INTERVAL       60000UL
static uint32_t heap_low_line_take (void) { return 0xFFFFFFFFUL; }
static size_t tcp_now_tcp_field (char * b, size_t n) { snprintf (b, n, "tcp=0/0/0/0/0/0"); return strlen (b); }
static uint16_t http_no_request_timeouts_get (void) { return 0; }
static void wifi_check_if_started (void) { }
static void http_server_loop (void) { }
static void http_ota_restart_poll (void) { }
static void update_cache_poll (void) { }
static void udp_server_loop (void) { }
static void ntp_poll_time (void) { }
static void var_sync_check (void) { }
static void stm_bridge_poll (void) { }
#include "heapkarte_aus.h"
#if ESP_HEAP_MAP
#error "heapkarte_aus.h steht nicht auf 0"
#endif
#include "heaplog.inc"
static void sim_loop (void)
{
#include "loop.inc"
}
int main (void)
{
    int f = 0;
    for (int i = 0; i < 30000; i++) { sim_loop (); sim_ms += 7; }
    int heap = 0, instr = 0;
    for (auto & z : zeilen) { if (! z.compare (0, 12, "- heap free=")) heap++; else if (! z.compare (0, 7, "- heap ")) instr++; }
    if (instr) { printf ("FEHLER ESP_HEAP_MAP 0: %d Zeilen des Instruments\n", instr); f = 1; }
    if (getfree != heap) { printf ("FEHLER ESP_HEAP_MAP 0: getFreeHeap %d mal, Heap-Zeilen %d - es wird noch gemessen\n", getfree, heap); f = 1; }
    if (heap != 4 || zeilen.empty () || zeilen[0] != "- heap free=7840 max=7104 ovr=0 low=7104 tcp=0/0/0/0/0/0 nrt=0")
    { printf ("FEHLER ESP_HEAP_MAP 0: Heap-Zeile \"%s\" (%d)\n", zeilen.empty () ? "" : zeilen[0].c_str (), heap); f = 1; }
    if (! f) printf ("  ESP_HEAP_MAP 0: keine Zeile, kein Messpunkt, Heap-Zeile unveraendert (%d Zeilen)\n", heap);
    return f;
}
