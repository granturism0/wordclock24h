/* Pruefstand c71e (Paket 2026-10-15, R1b, L7; AKS.1; design.md 7, 8).
 *
 * Echter Code: httpclient.cpp ganz (httpclient (), httpclient_stop (), httpclient_read (), httpclient_read_header (),
 * httpclient_wait_for_data ()). Attrappe: core_c71e.h (ClientContext mit Refcount und pbuf-Kette, WiFiClient mit der
 * Semantik des Core 3.1.2), basis.h aus c65 (String, Zeit, Pruefprotokoll).
 * Die Aufrufer aus http.cpp sind hier nachgebildet - in ihrer Form "len > 0 -> lesen oder nicht, httpclient_stop ();
 * sonst aufgeben", die alle elf Aufrufstellen teilen (c71e_static.py zaehlt sie). Gegenstand ist, dass JEDER Weg
 * den Kontext samt ungelesener Antwort freigibt.
 */
#include "basis.h"
#include "core_c71e.h"
#define HEAP_LOW_PATH_CLIENT 4
static int  tw_aufrufe = 0, heap_notes = 0;
static void tcp_tw_limit (void) { tw_aufrufe++; }
static void heap_low_note (uint8_t) { heap_notes++; }
static int  mystrnicmp (const char * a, const char * b, int n) { return strncasecmp (a, b, (size_t) n); }
#include "httpclient.h"                     /* echter Kopf des geprueften Stands (-I Quellverzeichnis) */
#include "hc.inc"

static long lebende (void) { long n = 0; for (auto * c : alle_kontexte) if (c->lebt) n++; return n; }
/* Antwort bauen: Kopf genau kopf_len Byte (mit Fuellzeile), dann der Rumpf; cl < 0: ohne Content-Length */
static Antwort antwort (int status, long cl, const std::string & rumpf, size_t kopf_len, std::vector<size_t> pbufs)
{
    std::string k = "HTTP/1.1 " + std::to_string (status) + " X\r\n";
    if (cl >= 0) k += "Content-Length: " + std::to_string (cl) + "\r\n";
    std::string fuell = "X-Fill: ";
    size_t rest = kopf_len > k.size () + fuell.size () + 4 ? kopf_len - k.size () - fuell.size () - 4 : 0;
    k += fuell + std::string (rest, 'f') + "\r\n\r\n";
    Antwort a; a.text = k + rumpf; a.pbufs = pbufs; return a;
}
static std::string stand (void) { return "lebend " + std::to_string (lebende ()) + ", heap " + std::to_string (heap_belegt) + ", tote Zugriffe " + std::to_string (tote_zugriffe); }
static bool frei (void) { return lebende () == 0 && heap_belegt == 0; }

int main (void)
{
    printf ("c71e (%s)\n", BEZEICHNUNG);
    const char * H = "srv.example", * P = "wordclock/test8";
    std::string icon_rumpf (1215, 'i');
    server["wc12h-icon.txt"]    = antwort (200, 1215, icon_rumpf, 250, { 536, 536, 393 });
    server["wc12h-weather.txt"] = antwort (200, 900, std::string (900, 'w'), 250, { 536, 614 });
    server["fehlt.txt"]         = antwort (404, 300, std::string (300, 'e'), 120, { 420 });
    server["leer.txt"]          = antwort (200, 0, "", 80, { 80 });
    server["ohne-laenge.txt"]   = antwort (200, -1, std::string (100, 'o'), 80, { 180 });
    server["wc-list.txt"]       = antwort (200, 3000, std::string (3000, 'l'), 150, { 536, 536, 536, 536, 536, 470 });
    server["wc.txt"]            = antwort (200, 7, "3.2.34\n", 100, { 107 });
    Antwort still; still.antwortet = false; server["schweigt.txt"] = still;

    /* (1) Existenzpruefung mit ungelesenem Rumpf, wie http_api_update_status (): Kopf 250 + 1'215 Byte in drei pbufs */
    {
        long kn = kontexte_neu, db = discard_bytes;
        int len = httpclient (H, P, "wc12h-icon.txt");
        fall ("(1) Rueckgabe wie bisher: Laenge des Rumpfs", len == 1215, std::to_string (len));
        fall ("(1) vor dem Schliessen liegt die Antwort im _rx_buf", lebende () == 1 && heap_belegt > 1215, stand ());
        if (len > 0) httpclient_stop ();                  /* ohne den Rumpf zu lesen */
        fall ("(1) nach httpclient_stop (): Refcount 0, Kontext frei, Heap wie vorher", frei () && kontexte_neu == kn + 1, stand ());
        fall ("(1) discard_received () gab den ungelesenen Rumpf frei (1'215 Byte)", discard_bytes - db == 1215, std::to_string (discard_bytes - db));
        int lw = httpclient (H, P, "wc12h-weather.txt"); if (lw > 0) httpclient_stop ();
        fall ("(1) zweite Pruefung (wc12h-weather.txt) ebenso frei", lw == 900 && frei (), stand ());
    }
    /* (2) Fehlerweg "keine Antwort" (Zeitgrenze im ersten Warten) */
    {
        unsigned long t0 = fake_ms;
        int len = httpclient (H, P, "schweigt.txt");
        fall ("(2) keine Antwort: -1 nach der Zeitgrenze", len == -1 && fake_ms - t0 >= 5000, std::to_string (len) + " nach " + std::to_string (fake_ms - t0) + " ms");
        fall ("(2) keine Antwort: Kontext frei", frei (), stand ());
    }
    /* (3) Status 404 mit Rumpf */
    {
        long db = discard_bytes;
        int len = httpclient (H, P, "fehlt.txt");
        fall ("(3) 404: -1 wie bisher", len == -1, std::to_string (len));
        fall ("(3) 404: Rumpf der Fehlerseite freigegeben, Kontext frei", frei () && discard_bytes - db == 300, stand () + ", verworfen " + std::to_string (discard_bytes - db));
    }
    /* (4) len == 0 (C75): 200 mit Content-Length 0 und 200 ohne Content-Length mit Rumpf */
    {
        int a = httpclient (H, P, "leer.txt");
        fall ("(4a) Content-Length 0: Rueckgabe 0, geschlossen und frei", a == 0 && frei (), std::to_string (a) + ", " + stand ());
        long db = discard_bytes;
        int b = httpclient (H, P, "ohne-laenge.txt");
        fall ("(4b) ohne Content-Length: Rueckgabe 0, Rumpf verworfen, frei", b == 0 && frei () && discard_bytes - db == 100, std::to_string (b) + ", " + stand ());
    }
    /* (5) Zweig zu-gross wie update_cache_write (): len > UPDATE_CACHE_LIST_MAX, schliessen ohne zu lesen */
    {
        int len = httpclient (H, P, "wc-list.txt");
        if ((size_t) len > 2048) httpclient_stop ();
        fall ("(5) zu-gross: Laenge wie bisher, nach httpclient_stop () frei", len == 3000 && frei (), std::to_string (len) + ", " + stand ());
    }
    /* (6) nach der Freigabe: weiterer Abruf verbindet normal, frischer Kontext, kein Zugriff auf alte */
    {
        long kn = kontexte_neu; int tw = tw_aufrufe; uint16_t hc = httpclient_connects_get ();
        int len = httpclient (H, P, "wc.txt");
        std::string r; int ch;
        while (len > 0 && (ch = httpclient_read (&len)) >= 0) r += (char) ch;
        fall ("(6) weiterer Abruf: neue Verbindung mit frischem Kontext", kontexte_neu == kn + 1 && tw_aufrufe == tw + 1 && httpclient_connects_get () == (uint16_t) (hc + 1), std::to_string (kontexte_neu - kn));
        fall ("(6) Anfrage und Antwort korrekt", r == "3.2.34\n" && len == 0 && letzte_anfrage == "GET /wordclock/test8/wc.txt HTTP/1.1\r\nHost: srv.example\r\nConnection: close\r\n\r\n", r);
        httpclient_stop ();
        fall ("(6) danach frei", frei (), stand ());
        fall ("(6) kein Zugriff auf einen freigegebenen Kontext (alle Faelle bis hier)", tote_zugriffe == 0, std::to_string (tote_zugriffe));
    }
    /* (7) Abruf, der zu Ende liest: wie vorher - Rueckgabe, Inhalt, Verbindung zu, kein Rest */
    {
        long db = discard_bytes, ds = discards;
        int len = httpclient (H, P, "wc12h-icon.txt"); int n0 = len;
        std::string r; int ch;
        while (len > 0 && (ch = httpclient_read (&len)) >= 0) r += (char) ch;
        fall ("(7) zu Ende gelesen: Inhalt vollstaendig", n0 == 1215 && r == icon_rumpf && len == 0, std::to_string (r.size ()));
        fall ("(7) die gelesenen pbufs sind schon beim Lesen frei, nur der Kontext lebt", lebende () == 1 && heap_belegt == CTX_BYTES, stand ());
        httpclient_stop ();
        fall ("(7) nach httpclient_stop (): frei, nichts zu verwerfen", frei () && discard_bytes == db && discards == ds, stand ());
        httpclient_stop ();
        fall ("(7) zweiter httpclient_stop () ist harmlos", frei () && tote_zugriffe == 0, stand ());
    }
    /* (8) Verbindungsaufbau scheitert: DNS (alter Kontext bliebe stehen) und Verbindung */
    {
        int len = httpclient (H, P, "wc.txt");             /* ein Aufrufer, der liest, aber nicht schliesst (gibt es heute nicht) */
        std::string r; int ch; while (len > 0 && (ch = httpclient_read (&len)) >= 0) r += (char) ch;
        fall ("(8) Vorbedingung: ein Kontext lebt", lebende () == 1, stand ());
        int d = httpclient ("dns-fehler", P, "wc.txt");
        fall ("(8) DNS scheitert: -1, alter Kontext freigegeben", d == -1 && frei (), std::to_string (d) + ", " + stand ());
        int v = httpclient ("verbindung-fehler", P, "wc.txt");
        fall ("(8) Verbindung scheitert: -1, frei", v == -1 && frei (), std::to_string (v) + ", " + stand ());
    }
    fall ("kein Zugriff auf einen freigegebenen Kontext (gesamt)", tote_zugriffe == 0, std::to_string (tote_zugriffe));
    return ende ("c71e", BEZEICHNUNG);
}
