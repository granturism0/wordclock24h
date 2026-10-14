/* Pruefstand c65m (Paket 2026-10-13, E3.4, AKR.4, AKR.6; design.md 18.5 (1)).
 * Echter Code beider Staende (alt = 3.2.35, neu = geprueft): http_api_update_table_files () samt Helfern,
 * http_api_update_status (), Zwischenspeicher und Sperrfenster, Pruefsumme. Attrappe: upd_harness.h.
 */
#include "upd_harness.h"

static std::string tabellenliste (int n24, int n12, const char * stamm)
{
    std::string r;
    for (int k = 0; k < n24; k++) r += std::string ("wc24h-tables-") + stamm + std::to_string (k) + ".txt\n";
    for (int k = 0; k < n12; k++) r += std::string ("wc12h-tables-") + stamm + std::to_string (k) + ".txt\n";
    return r + "README\n";
}
static int tab (void) { return abrufe["wc-list-tables.txt"]; }
static void job_bis_ruhe (void)
{
    for (int k = 0; k < 20; k++) { neu::update_cache_poll (); }
}

int main (void)
{
    printf ("c65m (%s)\n", BEZEICHNUNG);
    setze (UPDATE_HOST_VAR, "srv.example"); setze (UPDATE_PATH_VAR, "wordclock/test8"); setze (VERSION_STR_VAR, "3.2.34");
    /* E3.0 (L450): 16 Eintraege fuer die bekannte Hardware */
    server_stand ("3.2.36", "1.4.99", "3.2.34", "<p>Notes \"zitiert\"</p>\nZeile 2\r\n", "wc24h-ws2812-f411.hex\nwc12h-x.hex\n", tabellenliste (16, 4, "de-"));
    fake_ms = 100000;
    job_bis_ruhe ();                                     /* Dateien fuer neu anlegen (c65n prueft das im Einzelnen) */

    /* --- AKR.4: kalt, warm, abgelaufen, andere Quelle, Hardware, refresh, Sperrfenster */
    fake_ms = 200000; zaehler_null ();
    std::string k1 = antwort_neu_tabellen ();
    fall ("kalt: ein Abruf", tab () == 1, std::to_string (tab ()));
    fall ("kalt: 16 Eintraege", std::count (k1.begin (), k1.end (), ',') >= 15 && feldwert (k1, "table_files").find ("wc12h") == std::string::npos, feldwert (k1, "table_files"));
    std::string a1 = antwort_alt_tabellen ();
    fall ("kalt: byte-gleich mit 3.2.35", k1 == a1, "neu " + std::to_string (k1.size ()) + " / alt " + std::to_string (a1.size ()));
    fake_ms += 10000; zaehler_null ();
    std::string w1 = antwort_neu_tabellen ();
    fall ("warm: kein Abruf", tab () == 0, std::to_string (tab ()));
    fall ("warm: byte-gleiche Antwort", w1 == k1, w1);
    snprintf (aktuelle_tabelle, sizeof aktuelle_tabelle, "wc24h-tables-en.txt"); zaehler_null ();
    std::string w2 = antwort_neu_tabellen ();
    fall ("warm: current_table bleibt live", tab () == 0 && feldwert (w2, "current_table") == "wc24h-tables-en.txt", feldwert (w2, "current_table"));
    snprintf (aktuelle_tabelle, sizeof aktuelle_tabelle, "wc24h-tables-de.txt");
    fake_ms += 51000; zaehler_null (); antwort_neu_tabellen ();
    fall ("abgelaufen (61 s): Abruf", tab () == 1, std::to_string (tab ()));
    zaehler_null (); params["refresh"] = "1"; antwort_neu_tabellen (); params.clear ();
    fall ("refresh=1: Abruf", tab () == 1, std::to_string (tab ()));
    zaehler_null (); setze (UPDATE_HOST_VAR, "anders.example"); std::string q = antwort_neu_tabellen ();
    fall ("andere Quelle: Abruf beim neuen Host", tab () == 1 && gefragte_hosts.size () == 1 && gefragte_hosts[0] == "anders.example", std::to_string (tab ()));
    setze (UPDATE_HOST_VAR, "srv.example"); zaehler_null (); antwort_neu_tabellen ();
    fall ("zurueck zur alten Quelle: Abruf", tab () == 1, std::to_string (tab ()));
    zaehler_null (); hardware_configuration = HW_WC_12H; std::string h12 = antwort_neu_tabellen ();
    fall ("Hardware gewechselt: Abruf, anders gefiltert", tab () == 1 && feldwert (h12, "table_files").find ("wc24h") == std::string::npos && feldwert (h12, "table_files").find ("wc12h") != std::string::npos, feldwert (h12, "table_files"));
    zaehler_null (); hardware_configuration = 0xFFFF; std::string h65 = antwort_neu_tabellen ();
    fall ("Hardware 65535: kein Abruf, leere Liste (wie 3.2.35: ohne Filter keine Liste)", tab () == 0 && feldwert (h65, "table_files") == "[]", feldwert (h65, "table_files"));
    hardware_configuration = HW_WC_24H; zaehler_null (); antwort_neu_tabellen ();
    fall ("nach 65535 wieder 24h: Abruf (Pruefsumme der 12h-Liste passt nicht)", tab () == 1, std::to_string (tab ()));

    /* Sperrfenster: ein langsamer Fehlschlag eines anderen Abrufs stellt es scharf */
    fake_ms += 61000; zaehler_null ();
    server["ESP-WordClock.txt"].da = false; server["ESP-WordClock.txt"].kosten = 1500;
    params["refresh"] = "1"; antwort_neu_status (); params.clear ();
    server["ESP-WordClock.txt"].da = true; server["ESP-WordClock.txt"].kosten = 0;
    zaehler_null ();
    std::string sp = antwort_neu_tabellen ();
    fall ("Sperrfenster: kein Abruf, leere Liste", tab () == 0 && feldwert (sp, "table_files") == "[]", std::to_string (tab ()) + " " + feldwert (sp, "table_files"));
    fake_ms += 31000;

    /* abgerissener Abruf: leer statt Teilliste, kurzes Fenster */
    zaehler_null (); server["wc-list-tables.txt"].abbruch_nach = 100;
    std::string ab = antwort_neu_tabellen ();
    fall ("abgerissen: leere Liste statt Teilliste", tab () == 1 && feldwert (ab, "table_files") == "[]", feldwert (ab, "table_files"));
    server["wc-list-tables.txt"].abbruch_nach = -1;
    fake_ms += 10000; zaehler_null (); std::string ab2 = antwort_neu_tabellen ();
    fall ("abgerissen: innerhalb 30 s kein neuer Abruf, leer", tab () == 0 && feldwert (ab2, "table_files") == "[]", std::to_string (tab ()));
    fake_ms += 21000; zaehler_null (); std::string ab3 = antwort_neu_tabellen ();
    fall ("abgerissen: nach 30 s neu geholt, vollstaendig", tab () == 1 && ab3 == k1, std::to_string (tab ()));
    zaehler_null (); server["wc-list-tables.txt"].da = false; fake_ms += 61000; std::string f404 = antwort_neu_tabellen ();
    fall ("404: leere Liste", tab () == 1 && feldwert (f404, "table_files") == "[]", feldwert (f404, "table_files"));
    server["wc-list-tables.txt"].da = true; fake_ms += 31000;

    /* Liste ueber dem Puffer: nicht gekuerzt, nicht abgelegt, Zeile gedrosselt */
    server["wc-list-tables.txt"].inhalt = tabellenliste (40, 2, "eine-sehr-lange-endung-");
    zaehler_null (); std::string gr = antwort_neu_tabellen (); std::string gr_alt = antwort_alt_tabellen ();
    int n_eintraege = 0; { std::string t = feldwert (gr, "table_files"); for (size_t p = 0; (p = t.find ("wc24h-", p)) != std::string::npos; p++) n_eintraege++; }
    fall ("zu gross: vollstaendig ausgegeben (40), byte-gleich mit 3.2.35", n_eintraege == 40 && gr == gr_alt, std::to_string (n_eintraege));
    std::string zl; for (auto & l : Serial.lines) if (! l.compare (0, 24, "- update liste zu gross ")) zl = l;
    fall ("zu gross: Zeile in Mitschnitt und Ring", ! zl.empty () && std::find (ring.begin (), ring.end (), zl) != ring.end () && zl.compare (0, 44, "- update liste zu gross wc-list-tables.txt n") == 0, zl);
    zaehler_null (); fake_ms += 5000; antwort_neu_tabellen ();
    bool zeile2 = false; for (auto & l : Serial.lines) if (! l.compare (0, 24, "- update liste zu gross ")) zeile2 = true;
    fall ("zu gross: nicht abgelegt, naechster Aufruf holt wieder live", tab () == 1, std::to_string (tab ()));
    fall ("zu gross: Zeile gedrosselt (nicht erneut innerhalb 10 min)", ! zeile2, "");
    zaehler_null (); fake_ms += 600000; antwort_neu_tabellen ();
    zeile2 = false; for (auto & l : Serial.lines) if (! l.compare (0, 24, "- update liste zu gross ")) zeile2 = true;
    fall ("zu gross: nach 10 min wieder eine Zeile", zeile2, "");
    server["wc-list-tables.txt"].inhalt = tabellenliste (16, 4, "de-");

    /* --- AKR.6: Feldnamen, Reihenfolge, Typen gegen 3.2.35 aus derselben Attrappe */
    fake_ms += 700000; job_bis_ruhe ();
    zaehler_null (); std::string sn = antwort_neu_status (); std::string sa = antwort_alt_status ();
    std::vector<std::string> schn = schema (sn), scha = schema (sa);
    std::string diff;
    for (size_t k = 0; k < std::max (schn.size (), scha.size ()); k++)
        if (k >= schn.size () || k >= scha.size () || schn[k] != scha[k]) { diff = (k < scha.size () ? scha[k] : "-") + " / " + (k < schn.size () ? schn[k] : "-"); break; }
    fall ("AKR.6 update_status: gleiche Felder, Reihenfolge, Typen", schn == scha && schn.size () > 150, std::to_string (schn.size ()) + "/" + std::to_string (scha.size ()) + " erste Abweichung alt/neu: " + diff);
    fall ("AKR.6 update_status: Notes und STM-Liste wie 3.2.35", feldwert (sn, "release_notes") == feldwert (sa, "release_notes") && feldwert (sn, "stm32_files") == feldwert (sa, "stm32_files"),
          feldwert (sn, "release_notes") + " | " + feldwert (sn, "stm32_files"));
    std::string tn = antwort_neu_tabellen (), ta = antwort_alt_tabellen ();
    fall ("AKR.6 update_table_files: gleiche Felder, Reihenfolge, Typen", schema (tn) == schema (ta) && schema (tn).size () == 3, std::to_string (schema (tn).size ()));

    /* --- Summe je Paar Aufrufe (design.md 18.5): 3.2.35 8 kalt / 3 warm, E3 6 kalt / 0 warm */
    fake_ms += 70000; zaehler_null (); antwort_neu_status (); antwort_neu_tabellen (); int neu_kalt = abrufe_gesamt;
    fake_ms += 10000; zaehler_null (); antwort_neu_status (); antwort_neu_tabellen (); int neu_warm = abrufe_gesamt;
    fake_ms += 70000; zaehler_null (); antwort_alt_status (); antwort_alt_tabellen (); int alt_kalt = abrufe_gesamt;
    fake_ms += 10000; zaehler_null (); antwort_alt_status (); antwort_alt_tabellen (); int alt_warm = abrufe_gesamt;
    fall ("Paar kalt: 6 Abrufe (3.2.35: 8)", neu_kalt == 6 && alt_kalt == 8, "neu " + std::to_string (neu_kalt) + ", alt " + std::to_string (alt_kalt));
    fall ("Paar warm: 0 Abrufe (3.2.35: 3)", neu_warm == 0 && alt_warm == 3, "neu " + std::to_string (neu_warm) + ", alt " + std::to_string (alt_warm));
    return ende ("c65m", BEZEICHNUNG);
}
