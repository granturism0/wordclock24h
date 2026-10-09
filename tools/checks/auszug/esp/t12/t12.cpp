/* Pruefstand E.1 (L338 / L-Befund 200 ms, Paket 2026-10-09 A1): query_weather () aus weather.cpp.
 *
 * ZIELTYPEN (L256): millis () ist uint32_t wie am ESP. Zeit vergeht NUR in delay (), yield (),
 * hostByName (), connect () und stop () -- eine Leseschleife ohne Kontrollabgabe dreht deshalb auf der
 * Stelle und wird von der Schleifenwache gemeldet (AKE.5).
 *
 * Attrappe mit Core-3.1.2-Semantik (Pfade in den Kommentaren):
 *  - connect (host, port): hostByName (host, ip, _timeout) UND connect (ip) mit _timeout, je mit der
 *    vollen Frist (WiFiClient.cpp:130-137, :145-163, ClientContext.h:129-159)
 *  - connected (): ESTABLISHED || available () (WiFiClient.cpp:327-333)
 *  - readStringUntil (): timedRead wartet je Zeichen bis _timeout und prueft connected () NICHT
 *    (Stream.cpp:30-42, :255-263); Vorgabe _timeout 5000 (WiFiClient.cpp:80)
 *  - stop (): bis WIFICLIENT_MAX_FLUSH_WAIT_MS = 300 ms (WiFiClient.cpp:306-325), hier immer voll
 *  - hostByName (h, ip, t): haengt hoechstens t ms (ESP8266WiFiGeneric.cpp:611-680)
 *
 * Geprueft je Fall: genau eine Messzeile "- weather fc=.. ms=.. <ergebnis>" auf Serial UND im Logring,
 * gleich, mit erwartetem Ergebnis (AKE.1); Gesamtdauer <= 5100 ms (AKE.2); Parser-Eingabe gleich der
 * erwarteten Zeichenkette (AKE.3/AKE.4); keine Schleife ohne Kontrollabgabe (AKE.5); genau eine
 * Endzeile (AKE.6); kein appid, Ort, keine Koordinaten auf Serial oder im Logring.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <type_traits>

static uint32_t now;
static long spin;                                         /* Aufrufe ohne Zeitfortschritt */
static uint32_t spin_at;
static bool spin_fail;
static void tick () { if (now != spin_at) { spin_at = now; spin = 0; } else if (++spin > 200000) { spin_fail = true; now += 1000; } }   /* Wache meldet UND rueckt vor, damit der Lauf endet */
static uint32_t millis () { return now; }
static void delay (unsigned long ms) { now += ms; }
static void yield () { now += 1; }                       /* Core: Kontext abgeben; hier 1 ms */

struct String : std::string {
  String () {}
  String (const char * s) : std::string (s) {}
  String (const std::string & s) : std::string (s) {}
  String & operator= (const char * s) { std::string::operator= (s); return *this; }
  String & operator+= (const char * s) { append (s); return *this; }
  String & operator+= (const std::string & s) { append (s); return *this; }
  String & operator+= (char c) { push_back (c); return *this; }
};

/* Serial-Rekorder: vollstaendige Zeilen */
struct FakeSerial {
  std::string cur; std::vector<std::string> lines;
  void print (const char * s) { cur += s; }
  void print (const std::string & s) { cur += s; }
  void print (int v) { cur += std::to_string (v); }
  void println (const char * s) { cur += s; lines.push_back (cur); cur.clear (); }
  void println (const std::string & s) { println (s.c_str ()); }
  void println (int v) { print (v); println (""); }
  void flush () {}
} Serial;
static std::vector<std::string> logring;
static void stm32_log_append (const char * l) { logring.push_back (l); }
static void debugmsg (const char * m) { Serial.print ("- "); Serial.println (m); }
static unsigned char * convert_utf8_to_iso8859 (const unsigned char * s) { return (unsigned char *) s; }

struct IPAddress { uint32_t a = 0; };
struct Seg { uint32_t at; std::string s; };
static const uint32_t NIE = 0xFFFFFFFF;

struct Szenario {
  uint32_t dns_ms = 10; bool dns_haengt = false;
  uint32_t conn_ms = 15; bool conn_haengt = false; bool conn_abgewiesen = false;
  std::vector<Seg> segs;                                  /* Zeiten relativ zum print () */
  uint32_t close_at = 0;                                  /* relativ zum print (); NIE = schliesst nie */
  uint32_t tropfen = 0;                                   /* >0: nach den Segmenten je ein 'x' alle tropfen ms, ohne Ende */
};
static Szenario sz;

struct FakeWiFi {
  int hostByName (const char *, IPAddress & ip, uint32_t t) {
    if (sz.dns_haengt || sz.dns_ms > t) { now += t; return 0; }
    now += sz.dns_ms; ip.a = 1; return 1; }
} WiFi;

struct WiFiClient {
  unsigned long _timeout = 5000; bool open = false; uint32_t t0 = 0; std::string buf; size_t got = 0, pos = 0;
  uint32_t tropf_next = 0;
  void setTimeout (unsigned long t) { _timeout = t; }
  int connect (IPAddress, int) {
    if (sz.conn_abgewiesen) { now += 5; return 0; }
    if (sz.conn_haengt || sz.conn_ms > _timeout) { now += _timeout; return 0; }
    now += sz.conn_ms; open = true; t0 = NIE; buf.clear (); got = pos = 0; return 1; }
  int connect (const char * host, int port) {            /* Core: DNS UND Verbindung je mit _timeout */
    IPAddress ip; if (WiFi.hostByName (host, ip, _timeout)) return connect (ip, port); return 0; }
  void print (const std::string &) { t0 = now; si = 0; tropf_next = 0; }
  size_t si = 0;
  void pull () {                                          /* angekommene Bytes in den Puffer */
    if (t0 == NIE) return;
    while (si < sz.segs.size () && now >= t0 + sz.segs[si].at) { buf += sz.segs[si].s; si++; }
    if (sz.tropfen && si >= sz.segs.size () && ! bremse ()) {
      uint32_t base = sz.segs.empty () ? t0 : t0 + sz.segs.back ().at;
      if (! tropf_next) tropf_next = base + sz.tropfen;
      while (now >= tropf_next) { buf += 'x'; tropf_next += sz.tropfen; } } }
  int available () { tick (); pull (); return (int) (buf.size () - pos); }
  int read () { if (! available ()) return -1; return (unsigned char) buf[pos++]; }
  /* NOTBREMSE: Ab 60 s Pruefstandzeit nach dem Senden verstummt die Attrappe und schliesst; so endet
   * auch eine Fassung ohne Gesamtfrist (readStringUntil haelt einen Tropfer endlos fest) und schlaegt an,
   * statt den Pruefstand anzuhalten. */
  bool bremse () const { return t0 != NIE && now >= t0 + 60000; }
  bool closed () { if (bremse ()) return true; return sz.close_at != NIE && t0 != NIE && now >= t0 + sz.close_at && ! sz.tropfen; }
  int connected () { if (! open) return 0; return ! closed () || available (); }
  int timedRead () { uint32_t st = millis (); do { int c = read (); if (c >= 0) return c; if (_timeout == 0) return -1; yield (); } while (millis () - st < _timeout); return -1; }
  String readStringUntil (char term) { String r; int c = timedRead (); while (c >= 0 && c != term) { r += (char) c; c = timedRead (); } return r; }
  void stop () { now += 300; open = false; }
};

static int parse_weather (const char *, uint_fast8_t);
static int parse_weather_fc (const char *, uint_fast8_t);
#include "weather_code.inc"

static std::vector<std::string> parsed;                   /* Eingaben an die Parser */
/* Den ECHTEN Rueckgabewert durchreichen (M1: 0 keine, 1 Erfolg, 2 Fehler). Die alte Fassung ist void --
 * dort zaehlt nur, ob eine Zeile entstand (so wurde sie auch benutzt). */
template <class F> static int rufe (F f, const char * a, uint_fast8_t i) {
  size_t b = Serial.lines.size ();
  if constexpr (std::is_void_v<decltype (f (a, i))>) { f (a, i); return Serial.lines.size () > b; } else return f (a, i); }
static int parse_weather (const char * a, uint_fast8_t i)    { parsed.push_back (a); return rufe (real_parse_weather, a, i); }
static int parse_weather_fc (const char * a, uint_fast8_t i) { parsed.push_back (a); return rufe (real_parse_weather_fc, a, i); }

static int fails, n, checks;
static const char * APPID = "GEHEIMAPPID0123", * LON = "8.5417", * LAT = "47.3769";

static bool endzeile (const std::string & l) {
  return ! l.compare (0, 8, "WEATHER ") || ! l.compare (0, 11, "WEATHER_FC ") || ! l.compare (0, 6, "WICON ")
      || ! l.compare (0, 9, "WICON_FC ") || ! l.compare (0, 5, "ERROR"); }

#define PRUEF(cond, ...) do { checks++; if (! (cond)) { fails++; printf ("FEHL fall %d (%s): ", n, name); printf (__VA_ARGS__); printf ("\n"); } } while (0)

/* soll_parse: erwartete Parser-Eingabe, nullptr = Parser darf nicht laufen; sofort: Dauer ohne stop ()
 * hoechstens bis zum letzten Datenbyte + 5 ms */
static void fall (const char * name, Szenario s, int fc, int icon, const char * soll_erg, const char * soll_parse, bool city = false, bool sofort = false, const char * soll_end = nullptr)
{
  sz = s; n++; Serial = FakeSerial (); logring.clear (); parsed.clear (); openweather_client = WiFiClient ();
  now = 1000000; spin = 0; spin_at = 0; spin_fail = false; uint32_t start = now;
  char cbuf[32]; strcpy (cbuf, "Zuerich Altstadt");
  char a[32], lo[16], la[16]; strcpy (a, APPID); strcpy (lo, LON); strcpy (la, LAT);
  if (city) query_weather (a, NULL, NULL, cbuf, icon, fc); else query_weather (a, lo, la, NULL, icon, fc);
  uint32_t dauer = now - start;
  PRUEF (! spin_fail, "Schleife ohne Kontrollabgabe (AKE.5)");
  PRUEF (dauer <= 5100, "Dauer %u ms > 5100 (AKE.2)", (unsigned) dauer);
  /* AKE.1 */
  int mz = 0; std::string mzeile;
  for (auto & l : Serial.lines) if (! l.compare (0, 10, "- weather ")) { mz++; mzeile = l; }
  PRUEF (mz == 1, "%d Messzeilen statt 1 (AKE.1)", mz);
  PRUEF (logring.size () == 1 && logring[0] == mzeile, "Logring %zu Zeilen, gleich=%d (AKE.1)", logring.size (), (int) (! logring.empty () && logring[0] == mzeile));
  if (mz == 1) {
    int f = -1; unsigned long ms = 0; char erg[16] = "";
    int k = sscanf (mzeile.c_str (), "- weather fc=%d ms=%lu %15s", &f, &ms, erg);
    PRUEF (k == 3 && f == fc && ! strcmp (erg, soll_erg), "Messzeile '%s', erwartet fc=%d %s (AKE.1)", mzeile.c_str (), fc, soll_erg);
    PRUEF (k == 3 && ms <= dauer && ms + 2 >= dauer, "ms=%lu gegen gemessene Dauer %u (AKE.1)", ms, (unsigned) dauer);
  }
  /* AKE.6 */
  int ez = 0; std::string ezeile;
  for (auto & l : Serial.lines) if (endzeile (l)) { ez++; ezeile = l; }
  PRUEF (ez == 1, "%d Endzeilen statt 1 (AKE.6)", ez);
  if (ez == 1 && soll_end) PRUEF (ezeile == soll_end, "Endzeile '%s', erwartet '%s' (AKE.6/M1)", ezeile.c_str (), soll_end);
  if (ez == 1 && strcmp (soll_erg, "ok") && strcmp (soll_erg, "fehler")) PRUEF (ezeile == std::string ("ERROR weather ") + soll_erg, "Endzeile '%s' (AKE.6)", ezeile.c_str ());
  /* AKE.3/AKE.4 */
  if (soll_parse) PRUEF (parsed.size () == 1 && parsed[0] == soll_parse, "Parser bekam %zu Eingaben, erste %.40s... (AKE.3/4)", parsed.size (), parsed.empty () ? "-" : parsed[0].c_str ());
  else PRUEF (parsed.empty (), "Parser lief %zu mal, sollte nicht (AKE.6)", parsed.size ());
  if (sofort) {
    uint32_t letzt = 0; for (auto & g : s.segs) letzt = g.at > letzt ? g.at : letzt;
    uint32_t vor_print = s.dns_ms + s.conn_ms;
    PRUEF (dauer <= vor_print + letzt + 300 + 5, "nicht sofort fertig: %u ms, Daten bis %u ms (AKE.2)", (unsigned) dauer, (unsigned) (vor_print + letzt));
  }
  /* keine Werte */
  for (auto & l : Serial.lines) PRUEF (! strstr (l.c_str (), APPID) && ! strstr (l.c_str (), "Zuerich") && ! strstr (l.c_str (), LON) && ! strstr (l.c_str (), LAT), "Wert auf Serial: '%s'", l.c_str ());
  for (auto & l : logring) PRUEF (! strstr (l.c_str (), APPID) && ! strstr (l.c_str (), "Zuerich"), "Wert im Logring: '%s'", l.c_str ());
}

static std::string hdr_cl (size_t len) { return "HTTP/1.1 200 OK\r\nServer: openresty\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " + std::to_string (len) + "\r\nConnection: close\r\n\r\n"; }
static std::string hdr_ch () { return "HTTP/1.1 200 OK\r\nServer: openresty\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n"; }
static std::string hex (size_t v) { char b[16]; snprintf (b, sizeof b, "%zx", v); return b; }
static Szenario cl (const std::string & body, uint32_t at, std::vector<size_t> schnitte = {}, uint32_t abst = 30) {
  Szenario s; std::string all = hdr_cl (body.size ()) + body; size_t p = 0; uint32_t t = at;
  for (size_t c : schnitte) { s.segs.push_back ({t, all.substr (p, c - p)}); p = c; t += abst; }
  s.segs.push_back ({t, all.substr (p)}); s.close_at = t; return s; }

int main ()
{
  std::string now_json = "{\"coord\":{\"lon\":8.54,\"lat\":47.38},\"weather\":[{\"id\":800,\"main\":\"Clear\",\"description\":\"klarer Himmel\",\"icon\":\"01d\"}],\"main\":{\"temp\":12.6,\"humidity\":71},\"name\":\"Zurich\",\"cod\":200}";
  std::string fc_json = "{\"cod\":\"200\",\"message\":0,\"cnt\":9,\"list\":[";
  for (int i = 0; i < 9; i++) fc_json += std::string (i ? "," : "") + "{\"dt\":" + std::to_string (1760000000 + i * 10800) + ",\"main\":{\"temp\":" + std::to_string (10 + i) + ".4,\"feels_like\":9.1,\"temp_min\":8.2,\"temp_max\":11.3,\"pressure\":1018,\"humidity\":80},\"weather\":[{\"id\":500,\"main\":\"Rain\",\"description\":\"Leichter Regen\",\"icon\":\"10d\"}],\"clouds\":{\"all\":75},\"wind\":{\"speed\":2.1,\"deg\":240},\"visibility\":10000,\"pop\":0.4,\"sys\":{\"pod\":\"d\"},\"dt_txt\":\"2026-10-09 12:00:00\"}";
  fc_json += "],\"city\":{\"id\":2657896,\"name\":\"Zurich\"}}";
  std::string noicon = "{\"weather\":[{\"id\":800,\"description\":\"klar\"}],\"main\":{\"temp\":3.0},\"cod\":200}";

  /* --- AKE.4: gleiches Parseergebnis (alle innerhalb 200 ms vollstaendig da) --- */
  { Szenario s = cl (now_json, 40); fall ("Content-Length, ein Segment, Koerper ohne \\n", s, 0, 0, "ok", now_json.c_str (), false, true); }
  { std::string all = hdr_cl (now_json.size ()) + now_json; Szenario s = cl (now_json, 30, {50, all.size () - 60}, 40);
    fall ("Content-Length, drei Segmente", s, 0, 0, "ok", now_json.c_str (), false, true); }
  { Szenario s; s.segs.push_back ({50, hdr_ch () + hex (now_json.size ()) + "\r\n" + now_json + "\r\n0\r\n\r\n"}); s.close_at = 50;
    fall ("chunked, ein Segment", s, 0, 0, "ok", (now_json + "\r").c_str ()); }
  { Szenario s; std::string h = hdr_ch (); s.segs.push_back ({30, h}); s.segs.push_back ({70, hex (now_json.size ()) + "\r\n" + now_json.substr (0, 40)});
    s.segs.push_back ({120, now_json.substr (40) + "\r\n0\r\n\r\n"}); s.close_at = 120;
    fall ("chunked, drei Segmente", s, 0, 0, "ok", (now_json + "\r").c_str ()); }
  { std::string all = hdr_cl (fc_json.size ()) + fc_json; Szenario s = cl (fc_json, 40, {1460, 2920}, 20);
    fall ("Vorhersage, grosser Koerper, Content-Length", s, 1, 0, "ok", fc_json.c_str (), true, true); }
  { Szenario s; s.segs.push_back ({40, hdr_ch () + hex (fc_json.size ()) + "\r\n" + fc_json.substr (0, 1400)});
    s.segs.push_back ({80, fc_json.substr (1400) + "\r\n0\r\n\r\n"}); s.close_at = 80;
    fall ("Vorhersage, grosser Koerper, chunked", s, 1, 0, "ok", (fc_json + "\r").c_str ()); }
  { Szenario s = cl (now_json, 40); fall ("Icon heute", s, 0, 1, "ok", now_json.c_str ()); }
  { Szenario s = cl (fc_json, 40); fall ("Icon morgen", s, 1, 1, "ok", fc_json.c_str ()); }

  /* --- AKE.3: erstes Byte nach mehr als 200 ms --- */
  { Szenario s = cl (now_json, 250);  fall ("Antwort nach 250 ms", s, 0, 0, "ok", now_json.c_str (), false, true); }
  { Szenario s = cl (now_json, 1500); fall ("Antwort nach 1500 ms", s, 0, 0, "ok", now_json.c_str (), false, true); }
  { Szenario s = cl (now_json, 4000); fall ("Antwort nach 4000 ms", s, 1, 0, "ok", now_json.c_str (), false, true); }
  { std::string all = hdr_cl (now_json.size ()) + now_json; Szenario s = cl (now_json, 30, {40}, 230);
    fall ("Kopf in zwei Segmenten, zweites nach 260 ms", s, 0, 0, "ok", now_json.c_str (), false, true); }

  /* --- AKE.2: Sicherheitsnetz --- */
  { Szenario s; s.close_at = NIE; fall ("Server sendet nie, schliesst nie", s, 0, 0, "timeout", nullptr); }
  { Szenario s; s.segs.push_back ({50, hdr_cl (500)}); s.tropfen = 40; s.close_at = NIE;
    fall ("Server schliesst nie, sendet troepfchenweise", s, 0, 0, "timeout", nullptr); }
  { Szenario s; s.segs.push_back ({50, hdr_cl (500) + "{\"cod\":200"}); s.close_at = NIE;
    fall ("Koerper ohne \\n, Server schliesst nie", s, 0, 0, "timeout", nullptr); }
  { Szenario s; s.dns_haengt = true; fall ("DNS haengt", s, 0, 0, "dns", nullptr); }
  { Szenario s; s.conn_haengt = true; fall ("connect haengt", s, 0, 0, "connfail", nullptr); }
  { Szenario s; s.dns_ms = 3000; s.conn_haengt = true; fall ("DNS 3 s, dann connect haengt (keine Doppelzaehlung)", s, 0, 0, "connfail", nullptr); }
  { Szenario s; s.conn_abgewiesen = true; fall ("Verbindung abgewiesen", s, 0, 0, "connfail", nullptr); }
  { Szenario s; s.segs.push_back ({50, "HTTP/1.1 200 OK\r\nServer: x\r\nContent-Type: a"}); s.close_at = 50;
    fall ("Kopf ohne Leerzeile, Server schliesst", s, 0, 0, "leer", nullptr); }
  { Szenario s; s.segs.push_back ({50, "HTTP/1.1 200 OK\r\nServer: x\r\n"}); s.close_at = NIE;
    fall ("Kopf ohne Leerzeile, Server schliesst nie", s, 0, 0, "timeout", nullptr); }
  { Szenario s; s.segs.push_back ({100, hdr_cl (now_json.size ())}); s.segs.push_back ({4900, now_json}); s.close_at = 4900;
    fall ("Koerper erst nach 4,9 s", s, 0, 0, "timeout", nullptr); }

  /* --- AKE.6: leere Antworten haben genau eine Endzeile --- */
  { Szenario s; s.close_at = 30; fall ("Server schliesst ohne Antwort", s, 0, 0, "leer", nullptr); }
  { Szenario s; s.segs.push_back ({30, hdr_cl (0)}); s.close_at = 30; fall ("Kopf ohne Koerper", s, 0, 0, "leer", nullptr); }
  { Szenario s = cl (noicon, 40); fall ("Icon verlangt, Antwort ohne Icon", s, 0, 1, "leer", noicon.c_str ()); }
  { Szenario s = cl (noicon, 40); fall ("Icon morgen verlangt, Antwort ohne Icon", s, 1, 1, "leer", noicon.c_str ()); }

  /* --- M1 (Review E.5): Endzeile meldet einen Fehler => Messzeile "fehler", Endzeile UNVERAENDERT, kein ERROR weather --- */
  std::string e401 = "{\"cod\":401, \"message\": \"Invalid API key. Please see https://openweathermap.org/faq#error401 for more info.\"}";
  std::string e401fc = "{\"cod\":\"401\",\"message\":\"Invalid API key.\"}";
  std::string e404 = "{\"cod\":\"404\",\"message\":\"city not found\"}";
  std::string html = "<html><head><title>502 Bad Gateway</title></head><body>openresty</body></html>";
  { Szenario s = cl (e401, 40);   fall ("cod 401 (falsche appid), Wetter", s, 0, 0, "fehler", e401.c_str (), false, false, "WEATHER Wetter heute: Error 401"); }
  { Szenario s = cl (e401fc, 40); fall ("cod 401, Vorhersage", s, 1, 0, "fehler", e401fc.c_str (), false, false, "WEATHER_FC Wetter morgen: Error 401"); }
  { Szenario s = cl (html, 40);   fall ("Parse Error, Wetter", s, 0, 0, "fehler", html.c_str (), false, false, "WEATHER Wetter heute: Parse Error"); }
  { Szenario s = cl (html, 40);   fall ("Parse Error, Vorhersage", s, 1, 0, "fehler", html.c_str (), false, false, "WEATHER_FC Wetter morgen: Parse Error"); }
  { Szenario s = cl (e401, 40);   fall ("Icon heute, cod 401", s, 0, 1, "fehler", e401.c_str (), false, false, "WEATHER Wetter heute: Error 401"); }
  { Szenario s = cl (e404, 40);   fall ("Icon morgen, cod 404", s, 1, 1, "fehler", e404.c_str (), true, false, "WEATHER_FC Wetter morgen: Error 404"); }
  { Szenario s = cl (html, 40);   fall ("Icon heute, Parse Error", s, 0, 1, "fehler", html.c_str (), false, false, "WEATHER Wetter heute: Parse Error"); }
  /* Erfolgsform exakt, und der Grenzfall cod 200 ohne temp/description: Erfolgsform, also ok */
  { Szenario s = cl (now_json, 40); fall ("Erfolg Wetter, Endzeile exakt", s, 0, 0, "ok", now_json.c_str (), false, false, "WEATHER Wetter heute: 13 Grad, klarer Himmel"); }
  { Szenario s = cl (now_json, 40); fall ("Erfolg Icon, Endzeile exakt", s, 0, 1, "ok", now_json.c_str (), false, false, "WICON 01d"); }
  { Szenario s = cl (fc_json, 40);  fall ("Erfolg Icon morgen, Endzeile exakt", s, 1, 1, "ok", fc_json.c_str (), false, false, "WICON_FC 10d"); }
  { std::string leer200 = "{\"cod\":200}"; Szenario s = cl (leer200, 40);
    fall ("cod 200 ohne temp/description (Erfolgsform)", s, 0, 0, "ok", leer200.c_str (), false, false, "WEATHER Wetter heute: "); }

  printf ("%s (%s): %d Faelle, %d Pruefungen, %d Fehler\n", fails ? "FEHLGESCHLAGEN" : "OK", BEZEICHNUNG, n, checks, fails);
  return fails ? 1 : 0;
}
