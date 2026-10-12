/* Pruefstand ntp -- Attrappen fuer WiFiUDP, IPAddress, Serial, millis; Erwartung unabhaengig formuliert.
 * WiFiUDP bildet die Core-Semantik 3.1.2 nach: parsePacket() verwirft den ungelesenen Rest des vorigen Pakets
 * (UdpContext::next()), read() liefert hoechstens den Rest des aktuellen Pakets, flush() SENDET (endPacket).
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <deque>
#include "pruefstand.h"
#define uint_fast8_t  unsigned int
typedef uint8_t byte;
static uint16_t word (uint8_t h, uint8_t l) { return (uint16_t) ((h << 8) | l); }
static zp_millis fake_ms = 1000;
static zp_millis millis (void) { return fake_ms; }
/* IPAddress wie im Core 3.1.2 mit ip=lm2f (IPv4-only): ein 32-Bit-Wert, IPAddress () = IP_ANY_TYPE = 0.0.0.0,
 * operator== ueber ip_addr_cmp = Wertvergleich, isSet () = nicht 0.0.0.0 und nicht 255.255.255.255
 * (IPAddress.cpp:35-40). KEIN eigenes "any"-Merkmal: Genau das verdeckte in der ersten Fassung, dass
 * IPAddress () == IPAddress (0,0,0,0) gilt (Review E.6). */
struct IPAddress {
    uint32_t v;
    IPAddress () : v (0) {}
    IPAddress (int a, int b, int c, int d) : v ((uint32_t) a | ((uint32_t) b << 8) | ((uint32_t) c << 16) | ((uint32_t) d << 24)) {}
    bool operator== (const IPAddress & x) const { return v == x.v; }
    bool operator!= (const IPAddress & x) const { return v != x.v; }
    bool isSet (void) const { return v != 0 && v != 0xFFFFFFFFu; }
};
#define PSTR(s)     (s)
#define snprintf_P  snprintf
struct SerialStub {
    std::string out;
    void print (const char * s) { out += s; }
    void println (const char * s) { out += s; out += "\r\n"; }
    void println (unsigned long v) { out += std::to_string (v); out += "\r\n"; }
    void println (unsigned int v) { out += std::to_string (v); out += "\r\n"; }
    void println (int v) { out += std::to_string (v); out += "\r\n"; }
    void println (uint16_t v) { out += std::to_string (v); out += "\r\n"; }
    void flush (void) {}
} Serial;
static std::vector<std::string> logring;
void stm32_log_append (const char * l) { logring.push_back (l); }
struct Pkt { std::vector<uint8_t> d; IPAddress ip; uint16_t port; };
static int sent_packets = 0, flush_calls = 0, read_bytes_total = 0, short_read = 0;
static IPAddress sent_to;
struct WiFiUDP {
    std::deque<Pkt> q; Pkt cur; size_t off = 0; bool have = false;
    void begin (unsigned) {}
    unsigned localPort (void) { return 2421; }
    int parsePacket (void) {
        have = false;
        if (q.empty ()) return 0;
        cur = q.front (); q.pop_front (); off = 0; have = true;
        return (int) cur.d.size ();
    }
    int read (uint8_t * b, size_t n) {
        if (! have) return 0;
        size_t r = cur.d.size () - off; if (r > n) r = n;
        if (short_read && r > (size_t) short_read) r = short_read;
        memcpy (b, cur.d.data () + off, r); off += r; read_bytes_total += (int) r; return (int) r;
    }
    IPAddress remoteIP (void) { return have ? cur.ip : IPAddress (); }
    uint16_t remotePort (void) { return have ? cur.port : 0; }
    void beginPacket (IPAddress a, int) { sent_to = a; }
    void write (const uint8_t *, size_t) {}
    void endPacket (void) { sent_packets++; }
    void flush (void) { flush_calls++; endPacket (); }
};
static int ipstr_to_ipno (int * ipno, char * s) { return sscanf (s, "%d.%d.%d.%d", ipno, ipno + 1, ipno + 2, ipno + 3) == 4; }

#include "ntp_code.inc"

static int fails = 0, faelle = 0;
static void fehl (const char * fall, const std::string & was) { fails++; if (fails <= 400) printf ("      FEHLER [%s]: %s\n", fall, was.c_str ()); }
static std::string esc (const std::string & s) { std::string r; for (char c : s) { if (c == '\r') r += "\\r"; else if (c == '\n') r += "\\n"; else r += c; } return r; }

static const IPAddress SRV (192, 0, 2, 10);
static std::vector<uint8_t> pkt (int len, int li, int stratum, uint32_t tx, uint32_t txfrac = 0x12345678)
{
    std::vector<uint8_t> d (len, 0xEE);
    if (len > 0) d[0] = (uint8_t) ((li << 6) | (4 << 3) | 4);
    if (len > 1) d[1] = (uint8_t) stratum;
    for (int k = 0; k < 4 && 40 + k < len; k++) d[40 + k] = (uint8_t) (tx >> (24 - 8 * k));
    for (int k = 0; k < 4 && 44 + k < len; k++) d[44 + k] = (uint8_t) (txfrac >> (24 - 8 * k));
    return d;
}
static bool guard_intakt (void) { for (auto b : ntp_mem.guard) if (b != 0xC3) return false; return true; }

static unsigned long soll_cnt = 0;      /* unabhaengig mitgezaehlt */
/* grund == nullptr: Annahme erwartet, sonst Verwurf mit "<grund>" (z. B. "li=3"); zeit_vor: Drossel geoeffnet */
static void fall (const char * name, std::vector<uint8_t> d, IPAddress ip, uint16_t port, const char * grund, uint32_t tx_soll = 0, bool zeit_vor = true)
{
    faelle++;
    if (zeit_vor) fake_ms += 60000;
    memset (ntp_mem.guard, 0xC3, sizeof ntp_mem.guard);
    memset (ntp_mem.buf, 0x77, sizeof ntp_mem.buf);
    Serial.out.clear (); size_t lr = logring.size (); int rb = read_bytes_total; int fc = flush_calls; int sp = sent_packets;
    ntp_udp.q.push_back ({ d, ip, port });
    ntp_poll_time ();
    std::string o = Serial.out;
    if (! guard_intakt ()) fehl (name, "Waechterbereich hinter dem Puffer ueberschrieben");
    if (read_bytes_total - rb > NTP_PACKET_SIZE) fehl (name, "mehr als 48 Byte gelesen: " + std::to_string (read_bytes_total - rb));
    if (flush_calls != fc || sent_packets != sp) fehl (name, "flush()/endPacket() beim Empfang -- das SENDET im Core 3.1.2");
    if (! grund)
    {
        std::string s = "TIME " + std::to_string ((unsigned long) tx_soll) + "\r\n";
        if (o != s) fehl (name, "erwartet " + esc (s) + ", bekam " + esc (o));
        if (logring.size () != lr) fehl (name, "Annahme schreibt in den Logring: " + logring.back ());
    }
    else
    {
        soll_cnt++;
        std::string s = "- ntp verworfen #" + std::to_string (soll_cnt) + " " + grund;
        if (o.find ("TIME") != std::string::npos) fehl (name, "TIME an den STM trotz Verwurf: " + esc (o));
        if (o != s + "\r\n") fehl (name, "Mitschnitt: erwartet \"" + s + "\", bekam \"" + esc (o) + "\"");
        if (logring.size () != lr + 1 || logring.back () != s) fehl (name, "API-Ring: erwartet \"" + s + "\"" + (logring.size () > lr ? ", bekam \"" + logring.back () + "\"" : ", bekam nichts"));
        if (read_bytes_total - rb > 0 && (! strncmp (grund, "laenge", 6) || ! strcmp (grund, "absender") || ! strncmp (grund, "port", 4)))
            if (! short_read) fehl (name, "verworfenes Paket wurde gelesen (" + std::to_string (read_bytes_total - rb) + " Byte)");
    }
    /* Der Rest eines nicht gelesenen Pakets darf das naechste nicht stoeren: parsePacket() verwirft ihn */
}

int main (void)
{
    printf ("ntp (%s)\n", BEZEICHNUNG);
    const uint32_t T = 3969000000u;                     /* 2025-10 in NTP-Sekunden */

    /* vor der ersten Anfrage: kein Absender passt */
    fall ("vor Anfrage", pkt (48, 0, 2, T), SRV, 123, "absender");

    /* Review E.6: Quelle 0.0.0.0 vor der ersten Anfrage -- im IPv4-Bau ist ntp_server_ip dann IPAddress () == 0.0.0.0 */
    fall ("Quelle 0.0.0.0 vor der Anfrage", pkt (48, 0, 2, T), IPAddress (0, 0, 0, 0), 123, "absender");
    fall ("Quelle 0.0.0.0 vor der Anfrage, Default-IPAddress", pkt (48, 0, 2, T), IPAddress (), 123, "absender");
    {   /* "Zeitserver" 0.0.0.0 bzw. 255.255.255.255 angefragt: Antwort von genau dieser Quelle nie annehmen */
        char z0[] = "0.0.0.0"; ntp_get_time (z0); fake_ms += 60000;
        fall ("Server 0.0.0.0, Quelle 0.0.0.0", pkt (48, 0, 2, T), IPAddress (0, 0, 0, 0), 123, "absender");
        char zb[] = "255.255.255.255"; ntp_get_time (zb); fake_ms += 60000;
        fall ("Server Broadcast, Quelle 255.255.255.255", pkt (48, 0, 2, T), IPAddress (255, 255, 255, 255), 123, "absender");
    }
    char srv[] = "192.0.2.10";
    Serial.out.clear (); ntp_get_time (srv);
    if (Serial.out != "OK time\r\n" || sent_packets != 3 || ! (sent_to == SRV)) fehl ("Anfrage", "OK time / Paket an den Server erwartet: " + esc (Serial.out));
    faelle++;

    /* Annahme */
    fall ("gueltig", pkt (48, 0, 2, T), SRV, 123, nullptr, T);
    fall ("LI 1", pkt (48, 1, 1, T + 1), SRV, 123, nullptr, T + 1);
    fall ("LI 2", pkt (48, 2, 15, T + 2), SRV, 123, nullptr, T + 2);
    fall ("Stratum 1", pkt (48, 0, 1, T), SRV, 123, nullptr, T);
    fall ("Transmit 1 (Aera 1, keine Pivotrechnung im ESP)", pkt (48, 0, 2, 1), SRV, 123, nullptr, 1);
    fall ("Transmit 0xFFFFFFFF", pkt (48, 0, 2, 0xFFFFFFFFu), SRV, 123, nullptr, 0xFFFFFFFFu);
    fall ("Transmit 0x80000000", pkt (48, 0, 2, 0x80000000u), SRV, 123, nullptr, 0x80000000u);

    /* Plausibilitaet */
    fall ("LI 3", pkt (48, 3, 2, T), SRV, 123, "li=3");
    fall ("Stratum 0 (KoD)", pkt (48, 0, 0, T), SRV, 123, "stratum=0");
    fall ("Transmit 0", pkt (48, 0, 2, 0, 0), SRV, 123, "transmit=0");
    fall ("Transmit 0 mit Bruchteil", pkt (48, 0, 2, 0, 0xFFFFFFFFu), SRV, 123, "transmit=0");
    fall ("LI 3 und Stratum 0", pkt (48, 3, 0, 0), SRV, 123, "li=3");

    /* Absender */
    fall ("fremde IP", pkt (48, 0, 2, T), IPAddress (192, 0, 2, 11), 123, "absender");
    fall ("fremde IP 0.0.0.0", pkt (48, 0, 2, T), IPAddress (0, 0, 0, 0), 123, "absender");
    fall ("Port 124", pkt (48, 0, 2, T), SRV, 124, "port=124");
    fall ("Port 0", pkt (48, 0, 2, T), SRV, 0, "port=0");
    fall ("Port 2421", pkt (48, 0, 2, T), SRV, 2421, "port=2421");

    /* Laengen (L391) */
    fall ("47 Byte", pkt (47, 0, 2, T), SRV, 123, "laenge=47");
    fall ("49 Byte", pkt (49, 0, 2, T), SRV, 123, "laenge=49");
    fall ("68 Byte", pkt (68, 0, 2, T), SRV, 123, "laenge=68");
    fall ("1024 Byte", pkt (1024, 0, 2, T), SRV, 123, "laenge=1024");
    fall ("1 Byte", pkt (1, 0, 2, T), SRV, 123, "laenge=1");
    fall ("1472 Byte", pkt (1472, 0, 2, T), SRV, 123, "laenge=1472");
    fall ("nach Ueberlaenge wieder gueltig", pkt (48, 0, 2, T + 5), SRV, 123, nullptr, T + 5);
    short_read = 20;
    fall ("kurzes read()", pkt (48, 0, 2, T), SRV, 123, "laenge=20");
    short_read = 0;
    fall ("fremde IP mit 1024 Byte", pkt (1024, 0, 2, T), IPAddress (198, 51, 100, 1), 123, "absender");

    /* Drosselung: erste sofort, danach hoechstens eine je 60 s; #n zaehlt alle */
    {
        int f0 = fails;
        size_t l0 = logring.size ();
        fake_ms += 60000;
        for (int k = 0; k < 20; k++) { ntp_udp.q.push_back ({ pkt (68, 0, 2, T), SRV, 123 }); Serial.out.clear (); ntp_poll_time (); soll_cnt++; fake_ms += 2000; }
        /* 20 Pakete in 40 s: genau eine Zeile */
        if (logring.size () != l0 + 1) fehl ("Drossel", "20 Verwuerfe in 40 s ergaben " + std::to_string (logring.size () - l0) + " Zeilen statt 1");
        fake_ms += 19999;   /* 59'999 ms nach der ersten */
        ntp_udp.q.push_back ({ pkt (68, 0, 2, T), SRV, 123 }); ntp_poll_time (); soll_cnt++;
        if (logring.size () != l0 + 1) fehl ("Drossel", "bei 59'999 ms gemeldet");
        fake_ms += 1;
        ntp_udp.q.push_back ({ pkt (68, 0, 2, T), SRV, 123 }); ntp_poll_time (); soll_cnt++;
        std::string s = "- ntp verworfen #" + std::to_string (soll_cnt) + " laenge=68";
        if (logring.size () != l0 + 2 || logring.back () != s) fehl ("Drossel", "nach 60 s erwartet \"" + s + "\"");
        /* gueltige Antwort waehrend der Drossel: TIME kommt trotzdem */
        Serial.out.clear (); ntp_udp.q.push_back ({ pkt (48, 0, 2, T), SRV, 123 }); ntp_poll_time ();
        if (Serial.out != "TIME " + std::to_string ((unsigned long) T) + "\r\n") fehl ("Drossel", "gueltige Antwort waehrend der Drossel nicht weitergereicht");
        /* Wickeln der Millisekundenuhr (L256) */
        fake_ms = 0xFFFFFFFFu - 20000u; ntp_udp.q.push_back ({ pkt (68, 0, 2, T), SRV, 123 }); ntp_poll_time (); soll_cnt++;
        size_t l1 = logring.size ();
        fake_ms += 30000; ntp_udp.q.push_back ({ pkt (68, 0, 2, T), SRV, 123 }); ntp_poll_time (); soll_cnt++;
        if (logring.size () != l1) fehl ("Wickeln", "30 s nach der Meldung ueber die Grenze gemeldet");
        fake_ms += 30000; ntp_udp.q.push_back ({ pkt (68, 0, 2, T), SRV, 123 }); ntp_poll_time (); soll_cnt++;
        if (logring.size () != l1 + 1) fehl ("Wickeln", "60 s nach der Meldung ueber die Grenze nicht gemeldet");
        faelle += 7;
        if (fails != f0) printf ("  Drossel: %d Fehler\n", fails - f0);
    }
    /* Anfrage an einen anderen Server: der alte gilt nicht mehr */
    {
        char s2[] = "192.0.2.20"; ntp_get_time (s2); fake_ms += 60000;
        fall ("alter Server nach Wechsel", pkt (48, 0, 2, T), SRV, 123, "absender");
        fall ("neuer Server", pkt (48, 0, 2, T), IPAddress (192, 0, 2, 20), 123, nullptr, T);
    }

    printf ("  Faelle: %d\n", faelle);
    printf ("%s  ntp %s: %d Fehler\n", fails ? "FEHL" : "OK  ", BEZEICHNUNG, fails);
    return fails ? 1 : 0;
}
