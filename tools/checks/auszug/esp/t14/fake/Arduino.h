/* Attrappe fuer den Pruefstand t14: gerade genug Arduino, um vars.cpp zu uebersetzen. */
#ifndef T14_ARDUINO_H
#define T14_ARDUINO_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <string>
typedef uint8_t byte;
struct String : std::string {
  String () {} String (const char * s) : std::string (s ? s : "") {} String (const std::string & s) : std::string (s) {}
  String (int v) : std::string (std::to_string (v)) {}
  const char * c_str () const { return std::string::c_str (); }
};
/* Serial-Rekorder: alles, was auf die STM-UART ginge, als Bytefolge */
struct T14Serial {
  std::string out;
  size_t write (const uint8_t * p, size_t n) { out.append ((const char *) p, n); return n; }
  size_t write (uint8_t c) { out.push_back ((char) c); return 1; }
  size_t print (const char * s) { out += s; return strlen (s); }
  size_t print (const String & s) { out += s; return s.size (); }
  size_t print (char c) { out.push_back (c); return 1; }
  size_t print (int v) { std::string s = std::to_string (v); out += s; return s.size (); }
  size_t print (unsigned int v) { std::string s = std::to_string (v); out += s; return s.size (); }
  size_t print (long v) { std::string s = std::to_string (v); out += s; return s.size (); }
  size_t print (unsigned long v) { std::string s = std::to_string (v); out += s; return s.size (); }
  size_t println (const char * s) { print (s); out += "\r\n"; return 0; }
  size_t println (const String & s) { print (s); out += "\r\n"; return 0; }
  size_t println (int v) { print (v); out += "\r\n"; return 0; }
  size_t println () { out += "\r\n"; return 0; }
  void flush () {}
  /* wie Print::printf () im Core 3.1.2 (Print.cpp:50-71) */
  size_t printf (const char * fmt, ...) __attribute__ ((format (printf, 2, 3))) {
    va_list a; va_start (a, fmt); char t[64]; char * b = t; size_t len = vsnprintf (t, sizeof t, fmt, a); va_end (a);
    if (len > sizeof (t) - 1) { b = new char[len + 1]; va_start (a, fmt); vsnprintf (b, len + 1, fmt, a); va_end (a); }
    write ((const uint8_t *) b, len); if (b != t) delete[] b; return len; }
  void end () {} void begin (unsigned long, int = 0) {} void swap () {}
};
extern T14Serial Serial;
extern uint32_t t14_now;
static inline uint32_t millis () { return t14_now; }
static inline void delay (unsigned long ms) { t14_now += ms; }
static inline void yield () {}
#define F(x) (x)
#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT 0
#define SERIAL_8E1 1
#define SERIAL_8N1 0
#endif
