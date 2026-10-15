/* Pruefstand c65n (Paket 2026-10-13, E3.4a, AKR.5, AKR.6; design.md 18.5a).
 * Echter Code: update_cache_* (Auftrag, Dateien, Ausliefern), http_json_text_put (), http_api_update_status ()
 * beider Staende, Sperrfenster, Pruefsumme, http_fetch_remote_line (), app_asset_storage_filename ().
 * LittleFS-Attrappe im Speicher (Platz in Bloecken, Schreibfehler), Update-Server mit Zaehlern, Zeit, WLAN.
 */
#include "upd_harness.h"

static const char * HOST = "srv.example";
static const char * PFAD = "wordclock/test8";
static unsigned quelle (const char * h, const char * p)          /* unabhaengig nachgebaut: 31er-Summe ohne Hardware */
{
    uint16_t s = 0; for (; *h; h++) s = (uint16_t) (s * 31u + (unsigned char) *h); s = (uint16_t) (s * 31u + '/');
    for (; *p; p++) s = (uint16_t) (s * 31u + (unsigned char) *p); return s;
}
static int max_je_durchlauf = 0;
static void poll (int n = 12)
{
    for (int k = 0; k < n; k++) { int v = abrufe_gesamt; neu::update_cache_poll (); max_je_durchlauf = std::max (max_je_durchlauf, abrufe_gesamt - v); }
}
static int zeilen (const char * teil) { int n = 0; for (auto & l : Serial.lines) if (l.find (teil) != std::string::npos) n++; return n; }
static bool ring_gleich (void) { for (auto & l : Serial.lines) if (! l.compare (0, 15, "- update cache ") && std::find (ring.begin (), ring.end (), l) == ring.end ()) return false; return true; }
static bool keine_tmp (void) { for (auto & e : LittleFS.f) if (e.first.find (".tmp") != std::string::npos) return false; return true; }
static std::string datei (const char * n) { return LittleFS.f.count (n) ? LittleFS.f[n] : std::string ("(fehlt)"); }
static void faellig (void)
{
#ifdef NEU_HAVE_CACHE
    neu::update_cache_due = 1;
#endif
}
static void ruhig (void)
{
#ifdef NEU_HAVE_I3
    neu::http_request_seen = 0;
#endif
}
static std::string kopf (const char * esp, const char * app, const char * wc, size_t len, const char * h = HOST, const char * p = PFAD)
{
    char b[128]; snprintf (b, sizeof b, "U1 %s|%s|%s|%04x %zu\n", esp, app, wc, quelle (h, p), len); return b;
}

int main (void)
{
    printf ("c65n (%s)\n", BEZEICHNUNG);
    setze (UPDATE_HOST_VAR, HOST); setze (UPDATE_PATH_VAR, PFAD); setze (VERSION_STR_VAR, "3.2.34");
    std::string notes1 = "<h3>Release E3</h3>\r\n<p>Ein \"Zitat\", ein Backslash \\ und ein\tTab.</p>\n";
    notes1 += std::string ("Umlaute: \xc3\xa4\xc3\xb6\xc3\xbc, und ein Nullbyte: ") + '\0' + " danach weiter.\n";
    notes1 += std::string (300, 'x') + "\"" + std::string (130, 'y') + "\n";
    std::string liste1 = "wc24h-ws2812-f411.hex\r\nwc12h-ws2812-f103.hex\n  wc24h-sk6812-rgbw-f411.hex\nREADME\nwc24h-ohne-zeilenende.hex";
    server_stand ("3.2.36", "1.4.99", "3.2.34", notes1, liste1, "wc24h-tables-de.txt\n");
    LittleFS.f["/app-app.js.gz"] = std::string (163621, 'z');

    /* --- vor 60 s nach dem Start nichts */
    fake_ms = 30000; zaehler_null (); poll ();
    fall ("vor 60 s nach dem Start: kein Abruf", abrufe_gesamt == 0, std::to_string (abrufe_gesamt));

    /* --- Datei fehlt: Auftrag holt und schreibt, Kopfzeile stimmt, Startabruf 5 Verbindungen */
    fake_ms = 61000; zaehler_null (); max_je_durchlauf = 0; poll ();
    fall ("Datei fehlt: Notes geschrieben, Kopfzeile stimmt", datei ("/upd-releasenote.txt") == kopf ("3.2.36", "1.4.99", "3.2.34", notes1.size ()) + notes1,
          datei ("/upd-releasenote.txt").substr (0, 60));
    fall ("Datei fehlt: Liste ungefiltert geschrieben", datei ("/upd-wc-list.txt") == kopf ("3.2.36", "1.4.99", "3.2.34", liste1.size ()) + liste1, datei ("/upd-wc-list.txt").substr (0, 60));
    fall ("Startabruf: 5 Abrufe (3 Versionen, Notes, Liste)", abrufe_gesamt == 5 && abrufe["releasenote.html"] == 1 && abrufe["wc-list.txt"] == 1, std::to_string (abrufe_gesamt));
    fall ("je Durchlauf hoechstens ein Abruf", max_je_durchlauf == 1, std::to_string (max_je_durchlauf));
    fall ("Zeilen: je Datei 'geschrieben n=<Inhalt>'", zeilen (("- update cache upd-releasenote.txt geschrieben n=" + std::to_string (notes1.size ())).c_str ()) == 1 &&
          zeilen (("- update cache upd-wc-list.txt geschrieben n=" + std::to_string (liste1.size ())).c_str ()) == 1 && ohne_i6 (Serial.lines) == 2, Serial.lines.empty () ? "" : Serial.lines[0]);   /* R1.3: ohne I-6-Zeilen */
    fall ("Zeilen: Mitschnitt und Ring textgleich", ring_gleich () && ohne_i6 (ring) == 2, std::to_string (ring.size ()));
    fall ("keine Temporaerdatei", keine_tmp (), "");
    fall ("nie der Server des Ursprungsprojekts", std::find (gefragte_hosts.begin (), gefragte_hosts.end (), std::string ("uclock.de")) == gefragte_hosts.end (), "");

    /* --- update_status mit gueltiger Datei: 0 Abrufe von Notes und Liste, byte-gleich mit dem Live-Weg von 3.2.35 */
    fake_ms = 200000; zaehler_null ();
    std::string sn = antwort_neu_status ();
    int n_notes = abrufe["releasenote.html"], n_liste = abrufe["wc-list.txt"];
    std::string sa = antwort_alt_status ();
    fall ("update_status: 0 Abrufe von Notes und Liste", n_notes == 0 && n_liste == 0, std::to_string (n_notes) + "/" + std::to_string (n_liste));
    fall ("update_status: Antwort byte-gleich mit 3.2.35 (Maskierung, Filter)", sn == sa, "neu " + std::to_string (sn.size ()) + " / alt " + std::to_string (sa.size ()) + ": " + feldwert (sn, "stm32_files"));
    fall ("update_status: STM-Liste gefiltert, ohne Zeile ohne Zeilenende", feldwert (sn, "stm32_files") == "[\"wc24h-ws2812-f411.hex\",\"wc24h-sk6812-rgbw-f411.hex\"]", feldwert (sn, "stm32_files"));
    zaehler_null (); ruhig (); poll ();
    fall ("kalter update_status mit gleichem Schluessel: kein Auftrag", abrufe_gesamt == 0, std::to_string (abrufe_gesamt));

    /* --- Schluessel gleich (Neustart): 0 Abrufe von Notes und Liste, 0 Schreibvorgaenge */
    fake_ms += 60000; zaehler_null (); faellig (); poll ();
    fall ("Schluessel gleich: 0 Abrufe Notes/Liste", abrufe["releasenote.html"] == 0 && abrufe["wc-list.txt"] == 0 && abrufe_gesamt == 3, std::to_string (abrufe_gesamt));
    fall ("Schluessel gleich: 0 Schreibvorgaenge", LittleFS.oeffnen_w == 0 && LittleFS.umbenennen == 0, std::to_string (LittleFS.oeffnen_w));
    fall ("Schluessel gleich: je Datei 'unveraendert'", zeilen ("- update cache upd-releasenote.txt unveraendert") == 1 && zeilen ("- update cache upd-wc-list.txt unveraendert") == 1, "");

    /* --- Hardware wechselt auf 65535 (ESP-Neustart, L42): kein Neuschreiben */
    hardware_configuration = 0xFFFF; fake_ms += 60000; zaehler_null (); faellig (); poll ();
    fall ("Hardware 65535: kein Neuschreiben", LittleFS.oeffnen_w == 0 && zeilen ("unveraendert") == 2, std::to_string (LittleFS.oeffnen_w));
    zaehler_null (); std::string s65 = antwort_neu_status ();
    fall ("Hardware 65535: Notes aus der Datei, Liste leer, kein Abruf", feldwert (s65, "stm32_files") == "[]" && abrufe["wc-list.txt"] == 0 && abrufe["releasenote.html"] == 0 && feldwert (s65, "release_notes").size () > 100,
          feldwert (s65, "stm32_files"));
    hardware_configuration = HW_WC_24H;

    /* --- Schluessel anders: der kalte update_status liefert noch die alte Datei und setzt den Auftrag; genau ein Schreiben je Datei */
    std::string notes2 = "<p>Release E3b</p>\n"; std::string liste2 = "wc24h-neu-f411.hex\n";
    server_stand ("3.2.37", "1.4.99", "3.2.34", notes2, liste2, "wc24h-tables-de.txt\n");
    fake_ms += 61000; zaehler_null ();
    std::string vor = antwort_neu_status ();
    fall ("neuer Stand: Notes bleiben bis zum Auftrag die alten", feldwert (vor, "release_notes").find ("Release E3") != std::string::npos && abrufe["releasenote.html"] == 0, "");
    zaehler_null (); ruhig (); poll ();
    fall ("Schluessel anders: genau ein Schreiben je Datei", LittleFS.oeffnen_w == 2 && LittleFS.umbenennen == 2 && abrufe["releasenote.html"] == 1 && abrufe["wc-list.txt"] == 1,
          std::to_string (LittleFS.oeffnen_w) + " " + std::to_string (LittleFS.umbenennen));
    fall ("Schluessel anders: neuer Inhalt", datei ("/upd-releasenote.txt") == kopf ("3.2.37", "1.4.99", "3.2.34", notes2.size ()) + notes2, datei ("/upd-releasenote.txt").substr (0, 50));

    /* --- 5 s nach einer Anfrage nicht */
#ifdef NEU_HAVE_I3
    neu::http_request_seen = 1; neu::http_last_request_millis = fake_ms;
#endif
    fake_ms += 1000; zaehler_null (); faellig (); poll ();
    fall ("innerhalb 5 s nach einer Anfrage: kein Abruf", abrufe_gesamt == 0, std::to_string (abrufe_gesamt));
    fake_ms += 4000; zaehler_null (); poll ();
    fall ("5 s nach der Anfrage: Auftrag laeuft", abrufe_gesamt == 3, std::to_string (abrufe_gesamt));
    ruhig ();

    /* --- WLAN */
    fake_ms += 1000; zaehler_null (); faellig (); WiFi.st = 0; poll ();
    fall ("ohne WLAN: kein Abruf", abrufe_gesamt == 0, std::to_string (abrufe_gesamt));
    WiFi.st = WL_CONNECTED; wifi_ap_mode = 1; poll ();
    fall ("im AP-Betrieb: kein Abruf", abrufe_gesamt == 0, std::to_string (abrufe_gesamt));
    wifi_ap_mode = 0; poll ();

    /* --- Lesen bricht ab: Temporaerdatei geloescht, alte Datei unveraendert; 10 min Abstand */
    std::string notes3 = "<p>Release E3c, laenger als der Abbruch</p>\n";
    server_stand ("3.2.38", "1.4.99", "3.2.34", notes3, liste2, "wc24h-tables-de.txt\n");
    server["releasenote.html"].abbruch_nach = 10;
    std::string alt_notes = datei ("/upd-releasenote.txt");
    fake_ms += 61000; zaehler_null (); faellig (); poll ();
    fall ("abgerissen: alte Datei unveraendert", datei ("/upd-releasenote.txt") == alt_notes, datei ("/upd-releasenote.txt").substr (0, 40));
    fall ("abgerissen: Temporaerdatei geloescht", keine_tmp (), "");
    fall ("abgerissen: Zeile", zeilen ("- update cache upd-releasenote.txt fehlgeschlagen abgerissen") == 1 && ring_gleich (), Serial.lines.empty () ? "" : Serial.lines[0]);
    server["releasenote.html"].abbruch_nach = -1;
    fake_ms += 300000; zaehler_null (); poll ();
    fall ("nach Fehlschlag: innerhalb 10 min kein Abruf", abrufe_gesamt == 0, std::to_string (abrufe_gesamt));
    fake_ms += 300001; zaehler_null (); poll ();
    fall ("nach 10 min: neuer Versuch, Notes geschrieben", datei ("/upd-releasenote.txt") == kopf ("3.2.38", "1.4.99", "3.2.34", notes3.size ()) + notes3, std::to_string (abrufe_gesamt));

    /* --- Schreibfehler */
    server_stand ("3.2.39", "1.4.99", "3.2.34", notes1, liste1, "x\n");
    alt_notes = datei ("/upd-releasenote.txt"); std::string alt_liste = datei ("/upd-wc-list.txt");
    LittleFS.schreibbudget = 50;
    fake_ms += 61000; zaehler_null (); faellig (); poll ();
    fall ("Schreibfehler: alte Dateien unveraendert, keine Temporaerdatei", datei ("/upd-releasenote.txt") == alt_notes && datei ("/upd-wc-list.txt") == alt_liste && keine_tmp (), "");
    fall ("Schreibfehler: Zeile", zeilen ("fehlgeschlagen schreiben") >= 1, Serial.lines.empty () ? "" : Serial.lines[0]);
    LittleFS.schreibbudget = -1;

    /* --- Platzgrenze: nichts geschrieben */
    LittleFS.total = LittleFS.belegt () + 163621 + 8192;          /* weniger als Datei + app-app.js.gz + ein Block */
    fake_ms += 700000; zaehler_null (); poll ();
    fall ("Platz: nichts geschrieben, Zeile", LittleFS.oeffnen_w == 0 && zeilen ("- update cache upd-releasenote.txt fehlgeschlagen platz") == 1 && datei ("/upd-releasenote.txt") == alt_notes,
          std::to_string (LittleFS.oeffnen_w));
    LittleFS.total = 655360;
    fake_ms += 700000; zaehler_null (); poll ();
    fall ("Platz wieder da: geschrieben", datei ("/upd-releasenote.txt") == kopf ("3.2.39", "1.4.99", "3.2.34", notes1.size ()) + notes1, "");

    /* --- Inhalt zu gross: nicht abgelegt, nicht gekuerzt */
    std::string gross (3073, 'n'); std::string grossliste (2049, 'l');
    server_stand ("3.2.40", "1.4.99", "3.2.34", gross, grossliste, "x\n");
    alt_notes = datei ("/upd-releasenote.txt");
    fake_ms += 61000; zaehler_null (); faellig (); poll ();
    fall ("zu gross: Notes nicht abgelegt", datei ("/upd-releasenote.txt") == alt_notes && zeilen ("upd-releasenote.txt fehlgeschlagen zu-gross") == 1, "");
    fall ("zu gross: Liste nicht abgelegt", zeilen ("upd-wc-list.txt fehlgeschlagen zu-gross") == 1, "");
    server_stand ("3.2.40", "1.4.99", "3.2.34", std::string (3072, 'n'), std::string (2048, 'l'), "x\n");
    fake_ms += 700000; zaehler_null (); poll ();
    fall ("genau an der Grenze (3072/2048): abgelegt", zeilen ("geschrieben n=3072") == 1 && zeilen ("geschrieben n=2048") == 1, "");

    /* --- ungueltige Dateien: 0 Byte, falsche Laenge, ohne "U1 " */
    server_stand ("3.2.40", "1.4.99", "3.2.34", notes1, liste1, "x\n");
    fake_ms += 61000; faellig (); poll ();
    std::string gut_n = datei ("/upd-releasenote.txt"), gut_l = datei ("/upd-wc-list.txt");
    struct { const char * name; std::string inhalt; } kaputt[] = {
        { "0 Byte", "" }, { "falsche Laenge", gut_n + "x" }, { "zu kurz", gut_n.substr (0, gut_n.size () - 1) }, { "ohne U1", "U2" + gut_n.substr (2) }, { "ohne Kopfzeile", notes1 } };
    for (auto & k : kaputt)
    {
        LittleFS.f["/upd-releasenote.txt"] = k.inhalt; LittleFS.f["/upd-wc-list.txt"] = k.inhalt;
#ifdef NEU_HAVE_CACHE
        neu::update_cache_due = 0;
#endif
        fake_ms += 1000; zaehler_null ();
        std::string s = antwort_neu_status ();
        bool due = false;
#ifdef NEU_HAVE_CACHE
        due = neu::update_cache_due;
#endif
        fall ((std::string ("ungueltig (") + k.name + "): Notes leer, kein Abruf, Auftrag faellig").c_str (),
              feldwert (s, "release_notes") == "" && abrufe["releasenote.html"] == 0 && due, feldwert (s, "release_notes").substr (0, 30));
        fall ((std::string ("ungueltig (") + k.name + "): STM-Liste live").c_str (), abrufe["wc-list.txt"] == 1 && feldwert (s, "stm32_files").find ("wc24h-ws2812-f411.hex") != std::string::npos, feldwert (s, "stm32_files"));
    }
    LittleFS.f["/upd-releasenote.txt"] = gut_n; LittleFS.f["/upd-wc-list.txt"] = gut_l;
    {   /* nur die Notes-Datei ungueltig, die Liste gueltig: der Auftrag muss trotzdem faellig werden */
        LittleFS.f["/upd-releasenote.txt"] = "";
#ifdef NEU_HAVE_CACHE
        neu::update_cache_due = 0;
#endif
        fake_ms += 1000; zaehler_null (); std::string s = antwort_neu_status (); bool due = false;
#ifdef NEU_HAVE_CACHE
        due = neu::update_cache_due;
#endif
        fall ("nur Notes ungueltig: leer, kein Abruf, Auftrag faellig", feldwert (s, "release_notes") == "" && abrufe_gesamt == 0 + abrufe["ESP-WordClock.txt"] + abrufe["app-version.txt"] + abrufe["wc.txt"] + abrufe["wc24h-icon.txt"] + abrufe["wc24h-weather.txt"] && due,
              std::to_string (abrufe_gesamt));
        LittleFS.f["/upd-releasenote.txt"] = gut_n;
        LittleFS.f["/upd-wc-list.txt"] = "";
#ifdef NEU_HAVE_CACHE
        neu::update_cache_due = 0;
#endif
        fake_ms += 1000; zaehler_null (); s = antwort_neu_status (); due = false;
#ifdef NEU_HAVE_CACHE
        due = neu::update_cache_due;
#endif
        fall ("nur Liste ungueltig: Notes aus der Datei, Liste live, Auftrag faellig", feldwert (s, "release_notes").size () > 100 && abrufe["wc-list.txt"] == 1 && abrufe["releasenote.html"] == 0 && due, "");
        LittleFS.f["/upd-wc-list.txt"] = gut_l;
    }

    /* --- Host oder Pfad leer: kein Abruf, nie DEFAULT_UPDATE_HOST, "quelle-leer" einmal */
    setze (UPDATE_HOST_VAR, ""); fake_ms += 61000; zaehler_null (); faellig (); poll (); poll ();
    fall ("Host leer: kein Abruf", abrufe_gesamt == 0, std::to_string (abrufe_gesamt));
    fall ("Host leer: 'quelle-leer' je Datei genau einmal", zeilen ("upd-releasenote.txt fehlgeschlagen quelle-leer") == 1 && zeilen ("upd-wc-list.txt fehlgeschlagen quelle-leer") == 1, std::to_string (Serial.lines.size ()));
    zaehler_null (); antwort_neu_status (); int status_abrufe = abrufe_gesamt; zaehler_null (); poll ();
    fall ("Host leer: auch nach update_status (DEFAULT_UPDATE_HOST) fragt der Auftrag nicht", abrufe_gesamt == 0 && status_abrufe > 0, std::to_string (abrufe_gesamt));
    setze (UPDATE_HOST_VAR, HOST); setze (UPDATE_PATH_VAR, ""); fake_ms += 2000; zaehler_null (); faellig (); poll ();
    fall ("Pfad leer: kein Abruf", abrufe_gesamt == 0, std::to_string (abrufe_gesamt));
    setze (UPDATE_PATH_VAR, PFAD);

    /* --- Quelle gewechselt: Auftrag faellig, neue Dateien */
    fake_ms += 2000; poll ();
    zaehler_null (); setze (UPDATE_HOST_VAR, "zweit.example"); fake_ms += 2000; poll ();
    fall ("Host gewechselt: Auftrag schreibt fuer die neue Quelle", datei ("/upd-releasenote.txt").compare (0, kopf ("3.2.40", "1.4.99", "3.2.34", notes1.size (), "zweit.example").size (),
          kopf ("3.2.40", "1.4.99", "3.2.34", notes1.size (), "zweit.example")) == 0, datei ("/upd-releasenote.txt").substr (0, 50));
    setze (UPDATE_HOST_VAR, HOST); fake_ms += 2000; poll ();

    /* --- Sperrfenster */
    server["ESP-WordClock.txt"].da = false; server["ESP-WordClock.txt"].kosten = 1500;
    fake_ms += 61000; params["refresh"] = "1"; antwort_neu_status (); params.clear ();
    server["ESP-WordClock.txt"].da = true; server["ESP-WordClock.txt"].kosten = 0;
    zaehler_null (); faellig (); fake_ms += 1000; poll ();
    fall ("Sperrfenster: kein Abruf", abrufe_gesamt == 0, std::to_string (abrufe_gesamt));
    fake_ms += 30000; poll ();
    fall ("nach dem Sperrfenster: Auftrag laeuft", abrufe_gesamt >= 3, std::to_string (abrufe_gesamt));

    fall ("ueber alles: je Durchlauf hoechstens ein Abruf", max_je_durchlauf == 1, std::to_string (max_je_durchlauf));
    fall ("ueber alles: keine Temporaerdatei", keine_tmp (), "");
    fall ("ueber alles: LittleFS nach jedem Zugriff wieder ausgehaengt", LittleFS.gemountet == 0, std::to_string (LittleFS.gemountet));
    return ende ("c65n", BEZEICHNUNG);
}
