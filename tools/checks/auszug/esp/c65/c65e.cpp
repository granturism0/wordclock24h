/* Vorschlag esp-developer (E3.8): Gegenproben zu M1, M2, N4 und ms= im Auftrag update_cache_poll (). */
#include "upd_harness.h"

static const char * HOST = "srv.example";
static const char * PFAD = "wordclock/test8";
static int max_je_durchlauf = 0;
static void poll (int n = 12)
{
    for (int k = 0; k < n; k++) { int v = abrufe_gesamt; neu::update_cache_poll (); max_je_durchlauf = std::max (max_je_durchlauf, abrufe_gesamt - v); }
}
static int zeilen (const std::string & teil) { int n = 0; for (auto & l : Serial.lines) if (l.find (teil) != std::string::npos) n++; return n; }
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
#ifdef NEU_HAVE_ABGLEICH
    fall ("R00 beim Start ist der Abgleich vorgemerkt (Anfangswert)", neu::update_cache_force == 1 && neu::update_cache_due == 1, std::to_string (neu::update_cache_force));
#else
    fall ("R00 beim Start ist der Abgleich vorgemerkt (Anfangswert)", false, "kein update_cache_force - Stand vor E4.5a");
#endif
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
    antwort_neu_status ();                                         /* kalt ueber die Zeit: refresh=1 setzt seit E4.5a den Auftrag */
    fall ("M2: kalter update_status ohne app-version: kein Auftrag", neu::update_cache_due == 0, std::to_string (neu::update_cache_due));
    fake_ms += 61000; antwort_neu_status ();
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

    /* ---- E4.5a (Lead 14.10.2026): Abgleich beim ersten Durchgang nach dem Start und nach refresh=1 */
    {
        for (auto & e : server) { e.second.da = true; e.second.kosten = 0; e.second.abbruch_nach = -1; }
        std::string n1 = std::string ("<p>Release E4</p>\n") + std::string (2900, 'a') + "\n";   /* > 1024: Anfang in Stuecken kopiert */
        server["releasenote.html"].inhalt = n1; server["wc-list.txt"].inhalt = liste;
        auto start = [] () {                                        /* wie nach einem Neustart: Zustand des Auftrags */
            neu::update_cache_due = 1; neu::update_cache_step = 0; neu::update_cache_retry_wait = 0;
#ifdef NEU_HAVE_ABGLEICH
            neu::update_cache_force = 1;
#endif
        };
        fake_ms += 600001; zaehler_null (); neu::update_cache_due = 1; neu::update_cache_retry_wait = 0; poll ();
        fake_ms += 600001; zaehler_null (); neu::update_cache_due = 1; poll ();      /* eingeschwungen: Schluessel passt */
        fall ("R0 eingeschwungen: Datei traegt den Inhalt", datei ("/upd-releasenote.txt").find (n1) != std::string::npos, alle ());

        /* R1 gleicher Schluessel, anderer Inhalt GLEICHER Laenge (Abweichung mitten drin) */
        std::string n2 = n1; n2[2500] = 'b';
        server["releasenote.html"].inhalt = n2;
        fake_ms += 61000; zaehler_null (); start (); poll ();
        std::string d = datei ("/upd-releasenote.txt");
        fall ("R1 nach Start: Notes genau ein Abruf", abrufe["releasenote.html"] == 1, std::to_string (abrufe["releasenote.html"]));
        fall ("R1 nach Start: Notes 'geschrieben', Datei = neuer Inhalt samt Kopf", zeilen ("upd-releasenote.txt geschrieben n=" + std::to_string (n2.size ())) == 1 &&
              d.size () > n2.size () && d.compare (d.size () - n2.size (), n2.size (), n2) == 0 && d.compare (0, 3, "U1 ") == 0, alle ());
        fall ("R1 nach Start: Liste abgeglichen, unveraendert, nicht geschrieben", abrufe["wc-list.txt"] == 1 && zeilen ("upd-wc-list.txt unveraendert ms=") == 1 &&
              LittleFS.oeffnen_w == 1 && LittleFS.umbenennen == 1, std::to_string (LittleFS.oeffnen_w) + " " + alle ());
        fall ("R1 je Datei genau eine Zeile, keine Temporaerdatei", cache_zeilen () == 2 && keine_tmp (), alle ());

        /* R2 andere Laenge */
        std::string n3 = n2 + "<p>Nachtrag</p>\n"; server["releasenote.html"].inhalt = n3;
        fake_ms += 61000; zaehler_null (); start (); poll (); d = datei ("/upd-releasenote.txt");
        fall ("R2 andere Laenge: ein Abruf, geschrieben, Inhalt stimmt", abrufe["releasenote.html"] == 1 && zeilen ("upd-releasenote.txt geschrieben") == 1 &&
              d.compare (d.size () - n3.size (), n3.size (), n3) == 0, alle ());

        /* R3 Abweichung am ersten und am letzten Byte */
        for (size_t pos : { (size_t) 0, n3.size () - 1 })
        {
            std::string n4 = n3; n4[pos] = (char) (n4[pos] ^ 1); server["releasenote.html"].inhalt = n4;
            fake_ms += 61000; zaehler_null (); start (); poll (); d = datei ("/upd-releasenote.txt");
            fall (("R3 Abweichung an Byte " + std::to_string (pos) + ": geschrieben, Inhalt stimmt").c_str (), zeilen ("upd-releasenote.txt geschrieben") == 1 &&
                  d.size () >= n4.size () && d.compare (d.size () - n4.size (), n4.size (), n4) == 0, alle ());
            n3 = n4;
        }

        /* R4 gleicher Inhalt: abgeglichen, nichts zum Schreiben geoeffnet */
        fake_ms += 61000; zaehler_null (); start (); poll ();
        fall ("R4 gleicher Inhalt: je ein Abruf, beide 'unveraendert'", abrufe["releasenote.html"] == 1 && abrufe["wc-list.txt"] == 1 && zeilen ("unveraendert ms=") == 2, alle ());
        fall ("R4 gleicher Inhalt: kein Oeffnen zum Schreiben, kein Umbenennen", LittleFS.oeffnen_w == 0 && LittleFS.umbenennen == 0, std::to_string (LittleFS.oeffnen_w));

        /* R5 ohne Start und ohne refresh: Auftrag faellig (etwa kalter update_status), Schluessel passt -> kein Abruf */
        std::string n5 = n3; n5[10] = 'X'; server["releasenote.html"].inhalt = n5;
        fake_ms += 61000; zaehler_null (); neu::update_cache_due = 1; poll ();
        fall ("R5 ohne Abgleich: kein Abruf von Notes und Liste, alte Datei bleibt", abrufe["releasenote.html"] == 0 && abrufe["wc-list.txt"] == 0 && zeilen ("unveraendert ms=") == 2, alle ());

        /* R6 refresh=1: setzt den Auftrag, holt NICHT selbst; der naechste Durchgang gleicht ab */
        fake_ms += 61000; zaehler_null (); neu::update_cache_due = 0;
        params["refresh"] = "1"; antwort_neu_status (); params.clear ();
        fall ("R6 update_status?refresh=1 holt Notes nicht selbst", abrufe["releasenote.html"] == 0 && abrufe["wc-list.txt"] == 0, std::to_string (abrufe["releasenote.html"]));
        fall ("R6 update_status?refresh=1 setzt den Auftrag", neu::update_cache_due == 1, std::to_string (neu::update_cache_due));
        fake_ms += 6000; zaehler_null (); poll (); d = datei ("/upd-releasenote.txt");
        fall ("R6 naechster Durchgang: ein Abruf, geschrieben", abrufe["releasenote.html"] == 1 && zeilen ("upd-releasenote.txt geschrieben") == 1 &&
              d.compare (d.size () - n5.size (), n5.size (), n5) == 0, alle ());
        fake_ms += 61000; zaehler_null (); neu::update_cache_due = 1; poll ();
        fall ("R6 danach wieder ohne Abgleich", abrufe["releasenote.html"] == 0, std::to_string (abrufe["releasenote.html"]));

        /* R7 Abgleich reisst ab: alte Datei bleibt, nichts geschrieben, Abgleich bleibt vorgemerkt (nach 10 min) */
        std::string vor = datei ("/upd-releasenote.txt");
        server["releasenote.html"].abbruch_nach = 100;
        fake_ms += 61000; zaehler_null (); start (); poll ();
        fall ("R7 abgerissen: 'fehlgeschlagen abgerissen', Datei unveraendert, nichts geoeffnet", zeilen ("upd-releasenote.txt fehlgeschlagen abgerissen") == 1 &&
              datei ("/upd-releasenote.txt") == vor && LittleFS.oeffnen_w == 0 && keine_tmp (), alle ());
        server["releasenote.html"].abbruch_nach = -1;
        fake_ms += 600001; zaehler_null (); poll ();
        fall ("R7 nach 10 min: der Abgleich laeuft erneut", abrufe["releasenote.html"] == 1 && zeilen ("upd-releasenote.txt unveraendert") == 1, alle ());
        fall ("R ueber alles: LittleFS ausgehaengt", LittleFS.gemountet == 0, std::to_string (LittleFS.gemountet));
    }

#ifdef NEU_HAVE_I3
    fall ("upd_cache_max_ms: Maximum bleibt ueber spaetere schnelle Schritte", neu::req_max_upd_cache_ms == 5000, std::to_string (neu::req_max_upd_cache_ms));
#endif
    fall ("ueber alles: je Durchlauf hoechstens ein Abruf", max_je_durchlauf == 1, std::to_string (max_je_durchlauf));
    fall ("ueber alles: LittleFS ausgehaengt", LittleFS.gemountet == 0, std::to_string (LittleFS.gemountet));
    return ende ("c65e", BEZEICHNUNG);
#endif
}
