/* Pruefstand C47 + Ent-11 (Paket 2026-10-10, E.1): Legacy-Seite maskiert Werte.
 *
 * Teil A  AKX.1  input_field () - Wert im Attribut: keine rohen & < > " ' , jedes andere Byte unveraendert,
 *                dekodiert byte-gleich mit der Eingabe (ISO-8859-1-Byte, UTF-8, Emoji, leer, lang).
 * Teil B  AKX.1  Rundlauf: gespeicherter Wert -> input_field () -> Attribut wie ein Browser dekodiert ->
 *                Formular unveraendert abgesendet (application/x-www-form-urlencoded) -> '+' -> ' ' aus
 *                http () -> http_set_params () -> http_get_param () == gespeicherter Wert.
 * Teil C  AKX.2  select_field () - Optionstexte aus Tabellendatei/STM maskiert.
 * Teil D  AKX.3  Release Notes nach Ent-2 = R2: Tags entfernt, Text bleibt, '>' ausserhalb maskiert,
 *                <br> je Zeile; geteiltes Tag, Zeile ueber 128 Zeichen, Schlusszeile ohne '\n'.
 *                Invariante fuer jeden Fall: ohne die eigenen "<br>" kein '<' und kein '>'.
 * Teil E  Ent-11 DS18xx: IS_UP = 1 und Index 255 => "invalid".
 *
 * Gegenprobe (DIR-014): Der Ausgangsstand gibt '"' im Attribut roh aus (A, B), reicht <script> in den
 * Release Notes durch (D) und zeigt 255 als "127.5&deg;C" (E) => FEHL.
 * Typbreiten (L256): kein Ergebnis haengt an long/size_t-Breiten; Laengen sind < 2^16.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>

#define FS(x) x
static std::string out;
static void http_send (const char * s) { out += s; }
#define http_send_FS(x) http_send (x)

#include "c47_params.inc"
#include "c47_attr.inc"
#include "c47_fields.inc"

/* ---- Attrappen fuer den Release-Notes-Block ---- */
#define RELEASENOTE_HTML "releasenote.html"
static std::string feed; static size_t feedpos; static int stopped;
static int httpclient (const char *, const char *, const char *) { feedpos = 0; return (int) feed.size (); }
static int httpclient_read (int * lenp) { if (feedpos >= feed.size ()) return -1; (*lenp)--; return (unsigned char) feed[feedpos++]; }
static void httpclient_stop (void) { stopped++; }
static struct { void print (const char *) {} void println (const char *) {} } Serial;
#include "c47_notesfilter.inc"
static void notes_block (void)
{
    const char *    update_host = "h";
    const char *    update_path = "p";
    int             len;
#include "c47_notes.inc"
    (void) len;
}

/* ---- Attrappen fuer die DS18xx-Anzeige ---- */
enum { DS18XX_IS_UP_NUM_VAR, DS18XX_TEMP_INDEX_NUM_VAR };
static unsigned ds_up, ds_idx;
static unsigned get_numvar (int v) { return v == DS18XX_IS_UP_NUM_VAR ? ds_up : ds_idx; }
static std::string ds_text (unsigned up, unsigned idx)
{
    uint_fast8_t    temp_index;
    char            ds18xx_temp[16];
    ds_up = up; ds_idx = idx;
#include "c47_ds.inc"
    (void) temp_index;
    return ds18xx_temp;
}

static int n, fails;
static void fehl (const char * teil, int fall, const std::string & was)
{
    fails++; printf ("FEHL %s fall %d: %s\n", teil, fall, was.c_str ());
}
static std::string sichtbar (const std::string & s)
{
    std::string r; char b[8];
    for (unsigned char c : s) { if (c >= 0x20 && c < 0x7f) r += (char) c; else { snprintf (b, sizeof b, "\\x%02X", c); r += b; } }
    return r;
}

/* Browser: HTML-Zeichenreferenzen im Attribut dekodieren - die fuenf benannten und numerische.
 * Byteweise; die UTF-8-Dekodierung der Seite ist nicht Sache der Maskierung. */
static std::string html_decode (const std::string & a, bool * fremd)
{
    std::string r; size_t i = 0; *fremd = false;
    while (i < a.size ())
    {
        if (a[i] != '&') { r += a[i++]; continue; }
        size_t e = a.find (';', i);
        std::string ent = e == std::string::npos ? "" : a.substr (i, e - i + 1);
        if      (ent == "&amp;")  r += '&';
        else if (ent == "&lt;")   r += '<';
        else if (ent == "&gt;")   r += '>';
        else if (ent == "&quot;") r += '"';
        else if (ent == "&#39;")  r += '\'';
        else { *fremd = true; r += a[i++]; continue; }        // rohes '&' ohne eine der fuenf: Fehler der Maskierung
        i = e + 1;
    }
    return r;
}

/* Browser: Formular als application/x-www-form-urlencoded */
static std::string form_encode (const std::string & v)
{
    std::string r; char b[4];
    for (unsigned char c : v)
    {
        if (isalnum (c) || c == '*' || c == '-' || c == '.' || c == '_') r += (char) c;
        else if (c == ' ') r += '+';
        else { snprintf (b, sizeof b, "%%%02X", c); r += b; }
    }
    return r;
}

/* Teil A: input_field () */
static std::string attr_aus_input (const std::string & v, std::string * rest)
{
    out.clear ();
    input_field ("ticker", "", v.c_str (), 32, 20);
    const std::string kopf = "<input type=\"text\" id=\"ticker\" name=\"ticker\" value=\"";
    if (out.compare (0, kopf.size (), kopf) != 0) { *rest = "Kopf falsch: " + out; return ""; }
    size_t e = out.find ('"', kopf.size ());
    *rest = out.substr (e);
    return out.substr (kopf.size (), e - kopf.size ());
}
static void fall_a (const std::string & v)
{
    n++; std::string rest; std::string a = attr_aus_input (v, &rest);
    if (rest != "\" maxlength=\"32\" size=\"20\">") { fehl ("A", n, "Attribut endet vorzeitig, Rest '" + sichtbar (rest) + "' bei '" + sichtbar (v) + "'"); return; }
    for (char c : a) if (c == '<' || c == '>' || c == '\'' || c == '"') { fehl ("A", n, "rohes Sonderzeichen in '" + sichtbar (a) + "'"); return; }
    bool fremd; std::string d = html_decode (a, &fremd);
    if (fremd) { fehl ("A", n, "rohes & in '" + sichtbar (a) + "'"); return; }
    if (d != v) { fehl ("A", n, "dekodiert '" + sichtbar (d) + "' statt '" + sichtbar (v) + "'"); return; }
    /* jedes Byte ausser den fuenf steht unveraendert und in derselben Reihenfolge da */
    std::string ohne; for (char c : v) if (! strchr ("&<>\"'", c)) ohne += c;
    std::string ohne_a; bool in_ent = false;
    for (char c : a) { if (c == '&') in_ent = true; else if (in_ent) { if (c == ';') in_ent = false; } else ohne_a += c; }
    if (ohne != ohne_a) fehl ("A", n, "andere Bytes veraendert: '" + sichtbar (ohne_a) + "'");
}

/* Teil B: Rundlauf ueber das Legacy-Formular */
static void fall_b (const std::string & v)
{
    n++; std::string rest; std::string a = attr_aus_input (v, &rest);
    bool fremd; std::string browserwert = html_decode (a, &fremd);       // was der Browser im Feld zeigt und absendet
    std::string q = "ticker=" + form_encode (browserwert) + "&action=saveticker";
    char param[256]; strncpy (param, q.c_str (), 255); param[255] = '\0';
    char * p;
#include "c47_plus.inc"
    http_set_params (param);
    std::string zurueck = http_get_param ("ticker");
    if (zurueck != v) fehl ("B", n, "zurueck '" + sichtbar (zurueck) + "' statt '" + sichtbar (v) + "'");
    else if (strcmp (http_get_param ("action"), "saveticker")) fehl ("B", n, "action verloren - Wert hat die Parameterliste zerlegt");
}

/* Teil C: select_field () */
static void fall_c (void)
{
    n++;
    const char * texte[3] = { "Mode <b>1</b>", "A&B", "Q\"uote'" };
    out.clear (); select_field ("dm", texte, 1, 0, 3, 0);
    for (int i = 0; i < 3; i++)
    {
        size_t s = out.find ("<option value=\"" + std::to_string (i) + "\"");
        size_t g = out.find ('>', s); size_t e = out.find ("</option>", g);
        if (s == std::string::npos || e == std::string::npos) { fehl ("C", n, "Option fehlt: " + out); return; }
        std::string t = out.substr (g + 1, e - g - 1);
        bool fremd; std::string d = html_decode (t, &fremd);
        if (t.find_first_of ("<>\"'") != std::string::npos || fremd || d != texte[i])
        { fehl ("C", n, "Optionstext roh oder falsch: '" + t + "'"); return; }
    }
}

/* Teil D: Release Notes */
static void fall_d (const std::string & ein, const std::string & soll)
{
    n++; out.clear (); feed = ein; notes_block ();
    std::string o = out, ohne_br = o; size_t k;
    while ((k = ohne_br.find ("<br>")) != std::string::npos) ohne_br.erase (k, 4);
    if (ohne_br.find_first_of ("<>") != std::string::npos) { fehl ("D", n, "Markup durchgereicht: '" + sichtbar (o.substr (0, 120)) + "'"); return; }
    if (o != soll) fehl ("D", n, "'" + sichtbar (o.substr (0, 160)) + "' statt '" + sichtbar (soll.substr (0, 160)) + "'");
}

/* Teil E: DS18xx */
static void fall_e (unsigned up, unsigned idx, const char * soll)
{
    n++; std::string t = ds_text (up, idx);
    if (t != soll) fehl ("E", n, "IS_UP " + std::to_string (up) + " Index " + std::to_string (idx) + ": '" + t + "' statt '" + soll + "'");
}

int main ()
{
    int na, nb, nc, nd, ne, f0;

    /* A */
    f0 = n;
    const char * a_faelle[] = {
        "a\"b<c>&'d", "\xE4", "Z\xC3\xBCrich", "\xC3\xA4", "\xF0\x9F\x98\x80", "", "x\"y<z", "&amp;", "&lt;b&gt;", "'einfach'",
        "\" onfocus=\"alert(1)", "<script>alert(1)</script>", "\x01\x7F\x80\xFF", "100% & mehr", "a+b c", "&#39;",
    };
    for (const char * v : a_faelle) fall_a (v);
    std::string lang64; for (int i = 0; i < 64; i++) lang64 += "a\"<&'\xC3\xBC>"[i % 8]; fall_a (lang64);
    std::string ampers (300, '&'); fall_a (ampers);                                   // 1500 Byte Ausgabe, viele Puffergrenzen
    std::string mix; for (int i = 0; i < 200; i++) mix += (char) (i % 2 ? '"' : (0x80 + i % 0x40)); fall_a (mix);
    na = n - f0;

    /* B */
    f0 = n;
    const char * b_faelle[] = {
        "x\"y<z", "&amp;", "Z\xC3\xBCrich", "\xF0\x9F\x98\x80 Wecker", "a+b c", "100%", "a&b=c", "\" & '", "<>",
        "Gr\xC3\xBC\xC3\x9F" "e", "\xE4", "D.M.Y", "        ",
    };
    for (const char * v : b_faelle) fall_b (v);
    fall_b (lang64.substr (0, 32));
    nb = n - f0;

    /* C */
    f0 = n; fall_c (); nc = n - f0;

    /* D */
    f0 = n;
    fall_d ("<script>alert(1)</script>\n", "alert(1)<br>\n");
    fall_d ("<img src=x onerror=alert(1)>Text\n", "Text<br>\n");
    fall_d ("<a href=\"javascript:alert(1)\">Link</a>\n", "Link<br>\n");
    fall_d ("Vorher <a\nhref=\"javascript:x\">Nachher\n", "Vorher <br>\nNachher<br>\n");      // ueber zwei Zeilen geteiltes Tag
    fall_d ("a > b &amp; c &lt;x&gt;\n", "a &gt; b &amp; c &lt;x&gt;<br>\n");
    fall_d ("Ende", "Ende<br>\n");                                                         // Schlusszeile ohne '\n'
    fall_d ("<!-- kommentar -->Text\n", "Text<br>\n");
    fall_d (std::string ("a\0b\n", 4), "ab<br>\n");
    fall_d ("x<script", "x<br>\n");                                                       // offenes Tag am Ende
    fall_d ("Zeile\r\n", "Zeile\r<br>\n");
    fall_d ("Gr\xC3\xBC\xC3\x9F" "e \xF0\x9F\x98\x80\n", "Gr\xC3\xBC\xC3\x9F" "e \xF0\x9F\x98\x80<br>\n");
    fall_d ("", "");
    fall_d ("<h1>Version 3.2</h1>\n<ul>\n<li>Fix &quot;A&quot;</li>\n</ul>\n", "Version 3.2<br>\n<br>\nFix &quot;A&quot;<br>\n<br>\n");
    std::string l300 (300, 'A'); fall_d (l300 + "\n", l300 + "<br>\n");                  // Zeile ueber 128 Zeichen
    std::string l120 (120, 'A'), b20 (20, 'B');
    fall_d (l120 + "<script>" + b20 + "</script>\n", l120 + b20 + "<br>\n");             // Tag ueber die 128-Byte-Grenze
    std::string viele; for (int i = 0; i < 100; i++) viele += ">"; fall_d (viele + "\n", [&] { std::string s; for (int i = 0; i < 100; i++) s += "&gt;"; return s + "<br>\n"; } ());
    std::string tag250 = "<a title=\"" + std::string (250, 'x') + "\">Ziel</a>\n"; fall_d (tag250, "Ziel<br>\n");
    nd = n - f0;

    /* E */
    f0 = n;
    fall_e (1, 255, "invalid");
    fall_e (1, 254, "127&deg;C");
    fall_e (1, 45, "22.5&deg;C");
    fall_e (1, 0, "0&deg;C");
    fall_e (0, 255, "offline");
    fall_e (0, 45, "offline");
    ne = n - f0;

    printf ("%s (%s): %d Faelle (A %d, B %d, C %d, D %d, E %d), %d Fehler\n", fails ? "FEHLGESCHLAGEN" : "OK", BEZEICHNUNG,
            n, na, nb, nc, nd, ne, fails);
    return fails ? 1 : 0;
}
