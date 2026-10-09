/* Pruefstand E.2 (Teil C, AKE.8): http_log_request () aus http.cpp mit Serial-Rekorder.
 * Faelle: Setter mit Wert, appid, GET /?a, ohne Query, nur '?', POST mit Query, ohne HTTP-Version,
 * ohne Leerzeichen, mit/ohne IP und Label. Geprueft: Zeile genau wie erwartet und KEIN Zeichen aus dem
 * Querystring (der Teil nach '?' bis zum Leerzeichen kommt in der Zeile nicht vor).
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <string>
struct String : std::string {
  String () {} String (const char * s) : std::string (s) {}
};
struct FakeSerial {
  std::string out;
  void print (const char * s) { out += s; }
  void print (const String & s) { out += s; }
  void write (const uint8_t * p, size_t n) { out.append ((const char *) p, n); }
  void println () { out += "\r\n"; }
  void flush () {}
} Serial;
#include "echo_code.inc"
static int n, fails;
static void fall (const char * ip, const char * label, const char * req, const char * soll)
{
  n++; Serial = FakeSerial ();
  http_log_request (String (ip), label, String (req));
  std::string erw = std::string (soll) + "\r\n";
  if (Serial.out != erw) { fails++; printf ("FEHL fall %d: '%s' => '%s', erwartet '%s'\n", n, req, Serial.out.c_str (), soll); return; }
  const char * q = strchr (req, '?');
  if (q && q[1] && q[1] != ' ') {
    std::string qs (q + 1, strcspn (q + 1, " "));
    if (Serial.out.find (qs) != std::string::npos) { fails++; printf ("FEHL fall %d: Querystring '%s' in der Zeile\n", n, qs.c_str ()); }
  }
  if (Serial.out.find ('=') != std::string::npos) { fails++; printf ("FEHL fall %d: '=' in der Zeile\n", n); }
}
int main ()
{
  fall ("192.0.2.20", "Safari", "GET /api/weather_city_set?city=Zuerich HTTP/1.1", "- request 192.0.2.20 [Safari]: GET /api/weather_city_set?...");
  fall ("192.0.2.20", "Chrome", "GET /api/weather_appid_set?appid=0123456789abcdef0123456789abcdef HTTP/1.1", "- request 192.0.2.20 [Chrome]: GET /api/weather_appid_set?...");
  fall ("192.0.2.20", 0, "GET /?a HTTP/1.1", "- request 192.0.2.20: GET /?...");
  fall ("192.0.2.20", "Firefox", "GET /app/ HTTP/1.1", "- request 192.0.2.20 [Firefox]: GET /app/");
  fall ("192.0.2.20", 0, "GET /api/status? HTTP/1.1", "- request 192.0.2.20: GET /api/status?...");
  fall ("192.0.2.20", 0, "POST /api/ticker_set?text=Hallo%20Welt&x=1 HTTP/1.1", "- request 192.0.2.20: POST /api/ticker_set?...");
  fall ("192.0.2.20", 0, "POST /upload HTTP/1.1", "- request 192.0.2.20: POST /upload");
  fall ("", 0, "GET /api/ticker_set?text=geheim", "- request: GET /api/ticker_set?...");
  fall ("192.0.2.1", 0, "GET /", "- request 192.0.2.1: GET /");
  fall ("192.0.2.1", 0, "GET", "- request 192.0.2.1: GET");
  fall ("192.0.2.1", 0, "/x?k=v", "- request 192.0.2.1: /x?...");
  fall ("192.0.2.1", 0, "", "- request 192.0.2.1: ");
  fall ("192.0.2.1", 0, "GET /api/a?b=c?d=e HTTP/1.1", "- request 192.0.2.1: GET /api/a?...");
  printf ("%s (%s): %d Faelle, %d Fehler\n", fails ? "FEHLGESCHLAGEN" : "OK", BEZEICHNUNG, n, fails); return fails ? 1 : 0;
}
