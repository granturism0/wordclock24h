/* Pruefstand c71i (Paket 2026-10-15, R1.3; AKI.3; design.md 3.3, 8).
 *
 * Echter Code aus http.cpp: der ganze Auftrag update_cache_* samt den I-6-Funktionen (update_cache_i6_put,
 * update_cache_heap_mark, update_cache_vorher, update_cache_heap_report, update_cache_tmp_mask, update_cache_tmp_note),
 * dazu http_accept_note () mit seinen Variablen; aus ESP-uclock.ino der Block mit ESP.getFlashChipId () aus setup ().
 * Attrappen: der Abgleich-Harness von c65 (upd_harness.h: Update-Server, LittleFS im Speicher, WLAN, Variablen) und
 * basis.h (ESP-Wertfolgen). Je Messpunkt haelt stats_haken fest, in welchem Zustand gemessen wurde: wie viele Abrufe
 * schon liefen, ob LittleFS gemountet ist, ob eine Verbindung des httpclient offen ist, wie viele Zeilen schon da sind.
 * Die tcp=-Zaehlung selbst prueft c72i; hier liefert tcp_now_tcp_field () einen einstellbaren Text.
 */
#include "upd_harness.h"
#include "c71have.inc"

namespace ea {
#include "ea.inc"
}
static void flash_block (void)
{
#include "flash.inc"
}

static const char * HOST = "srv.example";
static const char * PFAD = "wordclock/test8";

struct Messung { int abrufe; int gemountet; bool offen; size_t zeilen; };
static std::vector<Messung> messungen;
static int acc_bei = -1; static uint16_t acc_plus = 0;           /* bei Messung Nr. acc_bei kommen acc_plus Verbindungen an */
static void haken (void)
{
    if ((int) messungen.size () == acc_bei) http_accept_count = (uint16_t) (http_accept_count + acc_plus);
    messungen.push_back ({ abrufe_gesamt, LittleFS.gemountet, akt != nullptr, Serial.lines.size () });
}
static void poll (int n = 12) { for (int k = 0; k < n; k++) neu::update_cache_poll (); }
static int zeilen (const std::string & teil) { int n = 0; for (auto & l : Serial.lines) if (l.find (teil) != std::string::npos) n++; return n; }
static std::string zeile (const char * praefix) { for (auto & l : Serial.lines) if (! l.compare (0, strlen (praefix), praefix)) return l; return "(fehlt)"; }
static int index_von (const char * praefix) { for (size_t k = 0; k < Serial.lines.size (); k++) if (! Serial.lines[k].compare (0, strlen (praefix), praefix)) return (int) k; return -1; }
static bool ring_gleich_i6 (void)
{
    size_t n = 0;
    for (auto & l : Serial.lines) if (i6_zeile (l)) { n++; if (std::count (ring.begin (), ring.end (), l) != 1) return false; }
    size_t m = 0; for (auto & l : ring) if (i6_zeile (l)) m++;
    return n == m;
}
static std::string alle (void) { std::string s; for (auto & l : Serial.lines) s += l + " / "; return s; }
/* sieben Messpunkte mit unterscheidbaren Werten vorbereiten */
static void werte (uint32_t heap0, uint32_t blk0, uint8_t frag0, int n = 7)
{
    ESP.heap_seq.clear (); ESP.blk_seq.clear (); ESP.frag_seq.clear ();
    for (int k = 0; k < n; k++) { ESP.heap_seq.push_back (heap0 + k); ESP.blk_seq.push_back (blk0 + k); ESP.frag_seq.push_back ((uint8_t) (frag0 + k)); }
}
static void neu_durchgang (void)
{
    zaehler_null (); messungen.clear (); acc_bei = -1; acc_plus = 0;
    neu::update_cache_due = 1;
}

int main (void)
{
    printf ("c71i (%s)\n", BEZEICHNUNG);
#ifndef C71_HAVE_I6
    fall ("I-6 im Auftrag vorhanden (update_cache_vorher ...)", false, "Stand vor R1.3");
#endif
    setze (UPDATE_HOST_VAR, HOST); setze (UPDATE_PATH_VAR, PFAD); setze (VERSION_STR_VAR, "3.2.34");
    std::string notes = "<p>Release R1</p>\n", liste = "wc24h-ws2812-f411.hex\n";
    server_stand ("3.2.38", "1.4.101", "3.2.34", notes, liste, "wc24h-tables-de.txt\n");
    LittleFS.f["/app-app.js.gz"] = std::string (163621, 'z');
    ESP.stats_haken = haken;

    /* --- T1 erster Durchgang, fuenf Schritte (Dateien fehlen), liegengebliebene Notes-Temporaerdatei */
    {
        LittleFS.f["/upd-releasenote.tmp"] = std::string (100, 't');
        hc_attrappe = 0; http_accept_count = 65534; tcp_attrappe = "tcp=1/0/2/0/0/3";
        neu_durchgang (); acc_bei = 2; acc_plus = 3;               /* zwischen Schritt 1 und 2 kommen drei Verbindungen an */
        werte (40001, 30001, 11);
        fake_ms = 61000; poll ();
        fall ("T1 Durchgang lief: beide Dateien geschrieben", zeilen ("upd-releasenote.txt geschrieben") == 1 && zeilen ("upd-wc-list.txt geschrieben") == 1, alle ());
        fall ("T1 vorher genau einmal", zeilen ("- update cache vorher ") == 1, alle ());
        fall ("T1 vorher: up, free/blk, frag, tcp=, hc", zeile ("- update cache vorher ") == "- update cache vorher up=61 free=40001/30001 frag=11 tcp=1/0/2/0/0/3 hc=0", zeile ("- update cache vorher "));
        fall ("T1 sieben Messpunkte (v, 1-5, n)", messungen.size () == 7, std::to_string (messungen.size ()));
        bool vor1 = messungen.size () > 1 && messungen[0].abrufe == 0 && messungen[1].abrufe == 1;
        fall ("T1 v VOR dem ersten Abruf, 1 nach der ersten Versionszeile", vor1, messungen.empty () ? "" : std::to_string (messungen[0].abrufe));
        bool folge = messungen.size () == 7; for (int k = 1; folge && k <= 5; k++) folge = messungen[k].abrufe == k; if (folge) folge = messungen[6].abrufe == 5;
        fall ("T1 Messpunkte k nach dem k-ten Abruf, n nach dem letzten", folge, "");
        bool zu = messungen.size () == 7; for (auto & x : messungen) zu = zu && x.gemountet == 0 && ! x.offen;
        fall ("T1 jeder Messpunkt nach LittleFS.end () und httpclient_stop ()", zu, "");
        fall ("T1 heap: free/blk je Schritt, in der Reihenfolge", zeile ("- update cache heap ") ==
              "- update cache heap v=40001/30001 1=40002/30002 2=40003/30003 3=40004/30004 4=40005/30005 5=40006/30006 n=40007/30007", zeile ("- update cache heap "));
        fall ("T1 nach: frag je Messpunkt, tcp=, hc, acc als Differenz ueber den Umlauf", zeile ("- update cache nach ") ==
              "- update cache nach frag=11/12/13/14/15/16/17 tcp=1/0/2/0/0/3 hc=5 acc=3", zeile ("- update cache nach "));
        int iv = index_von ("- update cache vorher "), ih = index_von ("- update cache heap "), in = index_von ("- update cache nach ");
        int iw = index_von ("- update cache upd-wc-list.txt");
        fall ("T1 Reihenfolge: vorher, Dateizeilen, heap, unmittelbar danach nach", iv >= 0 && iv < iw && iw < ih && in == ih + 1, alle ());
        fall ("T1 tmp geloescht genau fuer die vorhandene Temporaerdatei", zeilen ("- update cache tmp geloescht ") == 1 &&
              zeile ("- update cache tmp geloescht ") == "- update cache tmp geloescht upd-releasenote.tmp" && ! LittleFS.exists ("/upd-releasenote.tmp"), alle ());
        fall ("T1 tmp-Zeile vor der vorher-Zeile (Loeschen gehoert zum Beginn des Durchgangs)", index_von ("- update cache tmp geloescht ") >= 0 && index_von ("- update cache tmp geloescht ") < iv, alle ());
        fall ("T1 jede I-6-Zeile genau einmal im Ring, Ring = Mitschnitt", ring_gleich_i6 (), "");
        bool kein_fehl = true; for (auto & l : Serial.lines) if (i6_zeile (l) && l.find ("fehlgeschlagen") != std::string::npos) kein_fehl = false;
        fall ("T1 keine I-6-Zeile enthaelt 'fehlgeschlagen'", kein_fehl, "");
    }
    /* --- T2 Durchgang mit drei Schritten: Schluessel passt, kein Abgleich angefordert */
    {
        neu_durchgang (); neu::update_cache_force = 0; hc_attrappe = 0; http_accept_count = 7;
        LittleFS.f["/upd-wc-list.tmp"] = "x";                      /* nicht der erste Durchgang: bleibt liegen, keine Zeile */
        werte (41001, 31001, 21);
        fake_ms += 60000; poll ();
        fall ("T2 Durchgang lief: beide unveraendert", zeilen ("unveraendert") == 2, alle ());
        fall ("T2 heap: 4 und 5 nicht gelaufen", zeile ("- update cache heap ") == "- update cache heap v=41001/31001 1=41002/31002 2=41003/31003 3=41004/31004 4=- 5=- n=41005/31005", zeile ("- update cache heap "));
        fall ("T2 nach: '-' fuer 4 und 5, acc=0 ohne Verbindung", zeile ("- update cache nach ") == "- update cache nach frag=21/22/23/24/-/-/25 tcp=1/0/2/0/0/3 hc=3 acc=0", zeile ("- update cache nach "));
        fall ("T2 nicht der erste Durchgang: keine tmp-Zeile, Datei bleibt", zeilen ("tmp geloescht") == 0 && LittleFS.exists ("/upd-wc-list.tmp"), alle ());
        LittleFS.f.erase ("/upd-wc-list.tmp");
    }
    /* --- T3 Abbruch an der ersten Versionszeile: ein Schritt */
    {
        neu_durchgang (); server["ESP-WordClock.txt"].da = false;
        werte (42001, 32001, 31);
        fake_ms += 60000; poll (3);
        fall ("T3 Fehlschlag: heap mit v, 1, n", zeile ("- update cache heap ") == "- update cache heap v=42001/32001 1=42002/32002 2=- 3=- 4=- 5=- n=42003/32003", zeile ("- update cache heap "));
        fall ("T3 nach: frag=31/32/-/-/-/-/33", zeile ("- update cache nach ").compare (0, 37, "- update cache nach frag=31/32/-/-/-/") == 0 && zeile ("- update cache nach ").find ("/-/33 ") != std::string::npos, zeile ("- update cache nach "));
        fall ("T3 heap/nach NACH den Zeilen 'fehlgeschlagen abruf'", index_von ("- update cache upd-wc-list.txt fehlgeschlagen") >= 0 && index_von ("- update cache upd-wc-list.txt fehlgeschlagen") < index_von ("- update cache heap "), alle ());
        server["ESP-WordClock.txt"].da = true;
    }
    /* --- T4 Abbruch mitten im Durchgang: Host wird leer (quelle-leer) */
    {
        fake_ms += 700000;                                         /* 10-min-Sperre nach T3 vorbei */
        neu_durchgang (); werte (43001, 33001, 41);
        poll (1);
        fall ("T4 nach Schritt 1 noch kein Bericht", zeilen ("- update cache heap ") == 0 && zeilen ("- update cache vorher ") == 1, alle ());
        setze (UPDATE_HOST_VAR, ""); fake_ms += 2000; poll (1);
        fall ("T4 Abbruch durch leeren Host: Bericht mit v, 1, n", zeile ("- update cache heap ") == "- update cache heap v=43001/33001 1=43002/33002 2=- 3=- 4=- 5=- n=43003/33003", alle ());
        poll (3);
        fall ("T4 danach kein zweiter Bericht", zeilen ("- update cache heap ") == 1 && zeilen ("- update cache nach ") == 1, alle ());
        setze (UPDATE_HOST_VAR, HOST); fake_ms += 2000; poll ();
    }
    /* --- T5 laengste Formen, Kappung auf 65535 */
    {
        neu_durchgang (); neu::update_cache_force = 1; LittleFS.f.erase ("/upd-releasenote.txt"); LittleFS.f.erase ("/upd-wc-list.txt");
        ESP.heap_seq.clear (); ESP.blk_seq.clear (); ESP.frag_seq.clear ();
        ESP.heap = 4294967295u; ESP.blk = 70000; ESP.frag = 100;
        hc_attrappe = 65530; http_accept_count = 1; acc_bei = 3; acc_plus = 65535; tcp_attrappe = "tcp=255/255/255/255/255/255";
        fake_ms = 4294967295ul - 600000ul; poll ();
        std::string v = zeile ("- update cache vorher "), h = zeile ("- update cache heap "), n = zeile ("- update cache nach ");
        fall ("T5 vorher <= 119 und vollstaendig", v.size () <= 119 && v == "- update cache vorher up=4294367 free=65535/65535 frag=100 tcp=255/255/255/255/255/255 hc=65530", std::to_string (v.size ()) + ": " + v);
        fall ("T5 heap <= 119, vollstaendig, auf 65535 gekappt", h.size () <= 119 &&
              h == "- update cache heap v=65535/65535 1=65535/65535 2=65535/65535 3=65535/65535 4=65535/65535 5=65535/65535 n=65535/65535", std::to_string (h.size ()) + ": " + h);
        fall ("T5 nach <= 119 und vollstaendig", n.size () <= 119 && n == "- update cache nach frag=100/100/100/100/100/100/100 tcp=255/255/255/255/255/255 hc=65535 acc=65535", std::to_string (n.size ()) + ": " + n);
        ESP.heap = 6000; ESP.blk = 5000; ESP.frag = 0; tcp_attrappe = "tcp=0/0/0/0/0/0"; fake_ms = 100000000ul;
    }
    /* --- T6 tmp geloescht: beide, keine (je als erster Durchgang) */
    {
#ifdef NEU_HAVE_CACHE
        neu_durchgang (); neu::update_cache_tmp_checked = 0; LittleFS.f["/upd-releasenote.tmp"] = "a"; LittleFS.f["/upd-wc-list.tmp"] = "b";
        fake_ms += 700000; poll ();
        fall ("T6 beide Temporaerdateien: je eine Zeile", zeilen ("- update cache tmp geloescht upd-releasenote.tmp") == 1 && zeilen ("- update cache tmp geloescht upd-wc-list.tmp") == 1 && ring_gleich_i6 (), alle ());
        neu_durchgang (); neu::update_cache_tmp_checked = 0; fake_ms += 700000; poll ();
        fall ("T6 keine Temporaerdatei: keine Zeile", zeilen ("tmp geloescht") == 0 && zeilen ("- update cache vorher ") == 1, alle ());
#endif
    }
    /* --- T7 erste angenommene Anfrage: genau eine Zeile, Zaehler je accept () */
    {
#ifdef C71_HAVE_EA
        zaehler_null (); ESP.heap_seq.clear (); ESP.blk_seq.clear (); ESP.heap = 7136; ESP.blk = 6192;
        uint16_t c0 = ea::http_accept_count;
        ea::http_accept_note (); ESP.heap = 1; ESP.blk = 1; ea::http_accept_note (); ea::http_accept_note ();
        fall ("T7 genau eine Zeile 'erste anfrage' mit dem Stand beim ersten accept ()", zeilen ("- heap erste anfrage") == 1 && zeile ("- heap erste anfrage") == "- heap erste anfrage free=7136 max=6192", alle ());
        fall ("T7 Mitschnitt und Ring textgleich", ring.size () == 1 && ring[0] == zeile ("- heap erste anfrage"), ring.empty () ? "(Ring leer)" : ring[0]);
        fall ("T7 http_accept_count je accept () + 1", (uint16_t) (ea::http_accept_count - c0) == 3, std::to_string (ea::http_accept_count - c0));
#else
        fall ("T7 http_accept_note () vorhanden", false, "Stand vor R1.3");
#endif
    }
    /* --- T8 Flash-Kennung */
    {
#ifdef C71_HAVE_FLASH
        zaehler_null (); ESP.flash_id = 0x1640EF; flash_block ();
        fall ("T8 Winbond: id 6 Hexziffern, vendor 2", Serial.lines.size () == 1 && Serial.lines[0] == "- flash id=1640ef vendor=ef" && ring.size () == 1 && ring[0] == Serial.lines[0], alle ());
        zaehler_null (); ESP.flash_id = 0xFF164085; flash_block ();
        fall ("T8 PUYA, oberstes Byte maskiert", Serial.lines.size () == 1 && Serial.lines[0] == "- flash id=164085 vendor=85", alle ());
        zaehler_null (); ESP.flash_id = 0x000001; flash_block ();
        fall ("T8 fuehrende Nullen", Serial.lines.size () == 1 && Serial.lines[0] == "- flash id=000001 vendor=01", alle ());
#else
        fall ("T8 Flash-Kennung in setup () vorhanden", false, "Stand vor R1.3");
#endif
    }
    return ende ("c71i", BEZEICHNUNG);
}
