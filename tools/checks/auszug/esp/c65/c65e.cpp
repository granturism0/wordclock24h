/* Vorschlag esp-developer (E3.8): Gegenproben zu M1, M2, N4 und ms= im Auftrag update_cache_poll (). */
#include "upd_harness.h"

static const char * HOST = "srv.example";
static const char * PFAD = "wordclock/test8";
static int max_je_durchlauf = 0;
static void poll (int n = 12)
{
    for (int k = 0; k < n; k++) { int v = abrufe_gesamt; neu::update_cache_poll (); max_je_durchlauf = std::max (max_je_durchlauf, abrufe_gesamt - v); }
}
static int zeilen (const char * teil) { int n = 0; for (auto & l : Serial.lines) if (l.find (teil) != std::string::npos) n++; return n; }
static int cache_zeilen (void) { int n = 0; for (auto & l : Serial.lines) if (! l.compare (0, 15, "- update cache ")) n++; return n; }
static bool alle_mit_ms (void) { for (auto & l : Serial.lines) if (! l.compare (0, 15, "- update cache ") && l.find (" ms=") == std::string::npos) return false; return true; }
static bool keine_tmp (void) { for (auto & e : LittleFS.f) if (e.first.find (".tmp") != std::string::npos) return false; return true; }
static std::string datei (const char * n) { return LittleFS.f.count (n) ? LittleFS.f[n] : std::string ("(fehlt)"); }
static std::string alle (void) { std::string s; for (auto & l : Serial.lines) s += l + " / "; return s; }

int main (void)
{
    printf ("c65e (%s)\n", BEZEICHNUNG);
#ifndef NEU_HAVE_CACHE
    printf ("KAPUTT: kein Auftrag im Stand\n"); return 2;
#else
    setze (UPDATE_HOST_VAR, HOST); setze (UPDATE_PATH_VAR, PFAD); setze (VERSION_STR_VAR, "3.2.34");
    std::string notes = "<p>Release E3</p>\n", liste = "wc24h-ws2812-f411.hex\n";
    server_stand ("3.2.36", "1.4.99", "3.2.34", notes, liste, "wc24h-tables-de.txt\n");
    LittleFS.f["/app-app.js.gz"] = std::string (163621, 'z');

    /* N4: liegengebliebene Temporaerdateien vor dem ersten Durchgang */
    LittleFS.f["/upd-releasenote.tmp"] = std::string (100, 't');
    LittleFS.f["/upd-wc-list.tmp"]     = std::string (50, 't');
    server["wc.txt"].da = false;                                   /* erster Durchgang schreibt nichts: nur N4 kann loeschen */
    fake_ms = 61000; zaehler_null (); poll ();
    fall ("N4: Temporaerdateien beim ersten Durchgang geloescht, ohne Schreiben", keine_tmp () && LittleFS.oeffnen_w == 0, std::to_string (LittleFS.oeffnen_w));
    server["wc.txt"].da = true;
    fake_ms += 600001; zaehler_null (); poll ();
    fall ("N4: danach laeuft der Durchgang durch", zeilen ("upd-releasenote.txt geschrieben n=") == 1 && zeilen ("upd-wc-list.txt geschrieben n=") == 1, alle ());
    fall ("ms=: jede Zeile traegt ms=", alle_mit_ms () && cache_zeilen () == 2, alle ());
    LittleFS.f["/upd-releasenote.tmp"] = "x";
    fake_ms += 61000; zaehler_null (); neu::update_cache_due = 1; poll ();
    fall ("N4: nur beim ersten Durchgang (spaeter keine Loeschung)", LittleFS.entfernen == 0 && LittleFS.f.count ("/upd-releasenote.tmp") == 1, std::to_string (LittleFS.entfernen));
    LittleFS.f.erase ("/upd-releasenote.tmp");

    /* M2: eine Versionszeile fehlt dauerhaft (404 sofort) - update_status setzt keinen Auftrag */
    fake_ms += 61000; zaehler_null ();
    server["app-version.txt"].da = false;
    neu::update_cache_due = 0;
    params["refresh"] = "1"; antwort_neu_status (); params.clear ();
    fall ("M2: kalter update_status ohne app-version: kein Auftrag", neu::update_cache_due == 0, std::to_string (neu::update_cache_due));
    fake_ms += 61000; params["refresh"] = "1"; antwort_neu_status (); params.clear ();
    fall ("M2: auch beim zweiten kalten Aufruf nicht", neu::update_cache_due == 0, std::to_string (neu::update_cache_due));

    /* M2: Auftrag mit fehlender Versionszeile schreibt nicht */
    std::string vor_n = datei ("/upd-releasenote.txt"), vor_l = datei ("/upd-wc-list.txt");
    fake_ms += 61000; zaehler_null (); neu::update_cache_due = 1; poll ();
    fall ("M2: eine Versionszeile fehlt => kein Schreiben", LittleFS.oeffnen_w == 0 && LittleFS.umbenennen == 0 && datei ("/upd-releasenote.txt") == vor_n && datei ("/upd-wc-list.txt") == vor_l,
          std::to_string (LittleFS.oeffnen_w));
    fall ("M2: kein Abruf nach dem Fehlschlag (wc.txt, Notes, Liste)", abrufe["ESP-WordClock.txt"] == 1 && abrufe["app-version.txt"] == 1 && abrufe_gesamt == 2, std::to_string (abrufe_gesamt));
    fall ("M2: je Datei 'fehlgeschlagen abruf'", zeilen ("upd-releasenote.txt fehlgeschlagen abruf ms=") == 1 && zeilen ("upd-wc-list.txt fehlgeschlagen abruf ms=") == 1 && cache_zeilen () == 2, alle ());
    server["app-version.txt"].da = true;
    fake_ms += 600001; zaehler_null (); poll ();
    fall ("M2: Server wieder vollstaendig: unveraendert, kein Schreiben", LittleFS.oeffnen_w == 0 && zeilen ("unveraendert ms=") == 2, alle ());

    /* M1: Server schlaeft - Schritt 1 scheitert langsam => genau ein Abruf, kein Schritt 2 */
    for (auto & e : server) { e.second.da = false; e.second.kosten = 5000; }
    fake_ms += 61000; zaehler_null (); neu::update_cache_due = 1; poll ();
    fall ("M1: Fehlschlag in Schritt 1 => genau ein Abruf", abrufe_gesamt == 1 && abrufe["ESP-WordClock.txt"] == 1, std::to_string (abrufe_gesamt));
    fall ("M1: Zeilen mit Dauer des Schritts", zeilen ("upd-releasenote.txt fehlgeschlagen abruf ms=5000") == 1 && zeilen ("upd-wc-list.txt fehlgeschlagen abruf ms=5000") == 1, alle ());
    fake_ms += 31000; zaehler_null (); poll ();
#ifdef NEU_HAVE_I3
    fall ("upd_cache_max_ms: langsamer Schritt 1 ergibt 5000", neu::req_max_upd_cache_ms == 5000, std::to_string (neu::req_max_upd_cache_ms));
#endif
    fall ("M1: nach dem Sperrfenster kein Schritt 2 (10-min-Sperre)", abrufe_gesamt == 0, std::to_string (abrufe_gesamt));
    fake_ms += 600000; zaehler_null (); poll ();
    fall ("M1: nach 10 min wieder genau ein Abruf", abrufe_gesamt == 1, std::to_string (abrufe_gesamt));

    /* M1: Sperrfenster wird waehrend des Durchgangs scharf (kalter update_status zwischen Schritt 1 und 2) */
    for (auto & e : server) { e.second.da = true; e.second.kosten = 0; }
    server["ESP-WordClock.txt"].inhalt = "3.2.37\n";
    fake_ms += 600001; zaehler_null (); neu::update_cache_due = 1; neu::update_cache_poll ();
    fall ("M1b: Schritt 1 gelaufen", abrufe_gesamt == 1 && neu::update_cache_step == 2, std::to_string (neu::update_cache_step));
    server["ESP-WordClock.txt"].da = false; server["ESP-WordClock.txt"].kosten = 5000;
    params["refresh"] = "1"; antwort_neu_status (); params.clear ();
    for (auto & e : server) { e.second.da = true; e.second.kosten = 0; }
    zaehler_null (); fake_ms += 1000; poll ();
    fall ("M1b: Sperrfenster mitten im Durchgang => Abbruch, kein Abruf", abrufe_gesamt == 0 && neu::update_cache_step == 0 && zeilen ("fehlgeschlagen abruf ms=0") == 2, alle ());
    fake_ms += 31000; zaehler_null (); poll ();
    fall ("M1b: nach dem Sperrfenster wartet die 10-min-Sperre", abrufe_gesamt == 0, std::to_string (abrufe_gesamt));

    /* M1: Notes scheitern am Server in Schritt 4 => Liste nicht mehr versucht */
    server["releasenote.html"].da = false; server["releasenote.html"].kosten = 5000;
    fake_ms += 600001; zaehler_null (); poll ();
    fall ("M1c: Notes 'abruf' => kein Abruf der Liste", abrufe["releasenote.html"] == 1 && abrufe["wc-list.txt"] == 0, std::to_string (abrufe_gesamt));
    fall ("M1c: beide Dateien je genau eine Zeile", zeilen ("upd-releasenote.txt fehlgeschlagen abruf ms=5000") == 1 && zeilen ("upd-wc-list.txt fehlgeschlagen abruf") == 1 && cache_zeilen () == 2, alle ());

    /* M1d (Lead, 14.10.2026): Notes scheitern SCHNELL (404, kein Sperrfenster). M1c scheitert langsam,
     * dort bricht schon das Sperrfenster in update_cache_poll() ab -- die Pruefung "w == 2" in Schritt 4
     * blieb so ungeprueft, die Sabotage daran gruen. Hier traegt sie allein. */
    for (auto & e : server) { e.second.da = true; e.second.kosten = 0; }
    server["releasenote.html"].da = false; server["releasenote.html"].kosten = 0;
    fake_ms += 600001; zaehler_null (); poll ();
    fall ("M1d: Notes scheitern schnell => trotzdem kein Abruf der Liste", abrufe["releasenote.html"] == 1 && abrufe["wc-list.txt"] == 0, std::to_string (abrufe["wc-list.txt"]));
    server["releasenote.html"].da = true;

#ifdef NEU_HAVE_I3
    fall ("upd_cache_max_ms: Maximum bleibt ueber spaetere schnelle Schritte", neu::req_max_upd_cache_ms == 5000, std::to_string (neu::req_max_upd_cache_ms));
#endif
    fall ("ueber alles: je Durchlauf hoechstens ein Abruf", max_je_durchlauf == 1, std::to_string (max_je_durchlauf));
    fall ("ueber alles: LittleFS ausgehaengt", LittleFS.gemountet == 0, std::to_string (LittleFS.gemountet));
    return ende ("c65e", BEZEICHNUNG);
#endif
}
